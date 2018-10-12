// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include <glm/geometric.hpp>

#include "emu.h"
#include "cpu/mb86233/mb86233.h"
#include "includes/model1.h"

#define LOG_TGP_VIDEO 0

#define LOG_TGP(x)  do { if (LOG_TGP_VIDEO) logerror x; } while (0)
#define LOG_TGP_DEV(d,x)  do { if (LOG_TGP_VIDEO) d->logerror x; } while (0)


// Model 1 geometrizer TGP and rasterizer simulation
enum { FRAC_SHIFT = 16 };
enum { MOIRE = 0x01000000 };

uint32_t model1_state::readi(int adr) const
{
	return m_display_list_current[(adr + 0)&0x7fff] | (m_display_list_current[(adr + 1)&0x7fff] << 16);
}

int16_t model1_state::readi16(int adr) const
{
	return m_display_list_current[(adr + 0)&0x7fff];
}

float model1_state::readf(int adr) const
{
	return u2f(readi(adr));
}

void model1_state::view_t::transform_point(point_t *p) const
{
	point_t q = *p;
	float xx = translation[0] * q.x + translation[3] * q.y + translation[6] * q.z + translation[ 9] + vxx;
		p->y = translation[1] * q.x + translation[4] * q.y + translation[7] * q.z + translation[10] + vyy;
	float zz = translation[2] * q.x + translation[5] * q.y + translation[8] * q.z + translation[11] + vzz;
	p->x = ayyc * xx - ayys * zz;
	p->z = ayys * xx + ayyc * zz;
}

void model1_state::view_t::transform_vector(glm::vec3& p) const
{
	glm::vec3 q(p);
	glm::vec3 row1(translation[0], translation[3], translation[6]);
	glm::vec3 row2(translation[1], translation[4], translation[7]);
	glm::vec3 row3(translation[2], translation[5], translation[8]);
	p = glm::vec3(glm::dot(q, row1), glm::dot(q, row2), glm::dot(q, row3));
	//p->set_x(translation[0] * q.x() + translation[3] * q.y() + translation[6] * q.z());
	//p->set_y(translation[1] * q.x() + translation[4] * q.y() + translation[7] * q.z());
	//p->set_z(translation[2] * q.x() + translation[5] * q.y() + translation[8] * q.z());
}

void model1_state::cross_product(point_t* o, const point_t* p, const point_t* q) const
{
	o->x = p->y * q->z - q->y * p->z;
	o->y = p->z * q->x - q->z * p->x;
	o->z = p->x * q->y - q->x * p->y;
}

float model1_state::view_determinant(const point_t *p1, const point_t *p2, const point_t *p3) const
{
	float x1 = p2->x - p1->x;
	float y1 = p2->y - p1->y;
	float z1 = p2->z - p1->z;
	float x2 = p3->x - p1->x;
	float y2 = p3->y - p1->y;
	float z2 = p3->z - p1->z;

	return p1->x * (y1 * z2 - y2 * z1) + p1->y * (z1 * x2 - z2 * x1) + p1->z * (x1 * y2 - x2 * y1);
}

void model1_state::view_t::project_point(point_t *p) const
{
	p->xx = p->x / p->z;
	p->yy = p->y / p->z;
	p->s.x = xc + (p->xx * zoomx + viewx);
	p->s.y = yc - (p->yy * zoomy + viewy);
}

void model1_state::view_t::project_point_direct(point_t *p) const
{
	p->xx = p->x /*/ p->z*/;
	p->yy = p->y /*/ p->z*/;
	p->s.x = xc + p->xx;
	p->s.y = yc - p->yy;
}

void model1_state::draw_hline(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color)
{
	uint32_t *base = &bitmap.pix32(y);
	while(x1 <= x2)
	{
		base[x1] = color;
		x1++;
	}
}

void model1_state::draw_hline_moired(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color)
{
	uint32_t *base = &bitmap.pix32(y);
	while(x1 <= x2)
	{
		if((x1^y) & 1)
		{
			base[x1] = color;
		}
		x1++;
	}
}

void model1_state::fill_slope(bitmap_rgb32 &bitmap, view_t *view, int color, int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t y1, int32_t y2, int32_t *nx1, int32_t *nx2)
{
	if(y1 > view->y2)
	{
		return;
	}

	if (y2 <= view->y1)
	{
		int delta = y2 - y1;
		*nx1 = x1 + delta * sl1;
		*nx2 = x2 + delta * sl2;
		return;
	}

	if (y2 > view->y2)
	{
		y2 = view->y2 + 1;
	}

	if (y1 < view->y1)
	{
		int delta = view->y1 - y1;
		x1 += delta * sl1;
		x2 += delta * sl2;
		y1 = view->y1;
	}

	if (x1 > x2 || (x1 == x2 && sl1 > sl2))
	{
		int32_t t = x1;
		x1 = x2;
		x2 = t;

		t = sl1;
		sl1 = sl2;
		sl2 = t;

		int32_t *tp = nx1;
		nx1 = nx2;
		nx2 = tp;
	}

	while(y1 < y2)
	{
		if(y1 >= view->y1)
		{
			int xx1 = x1>>FRAC_SHIFT;
			int xx2 = x2>>FRAC_SHIFT;
			if(xx1 <= view->x2 || xx2 >= view->x1)
			{
				if(xx1 < view->x1)
				{
					xx1 = view->x1;
				}
				if(xx2 > view->x2)
				{
					xx2 = view->x2;
				}

				if(color & MOIRE)
				{
					draw_hline_moired(bitmap, xx1, xx2, y1, color);
				}
				else
				{
					draw_hline(bitmap, xx1, xx2, y1, color);
				}
			}
		}

		x1 += sl1;
		x2 += sl2;
		y1++;
	}
	*nx1 = x1;
	*nx2 = x2;
}

void model1_state::fill_line(bitmap_rgb32 &bitmap, view_t *view, int color, int32_t y, int32_t x1, int32_t x2)
{
	int xx1 = x1>>FRAC_SHIFT;
	int xx2 = x2>>FRAC_SHIFT;

	if(y > view->y2 || y < view->y1)
		return;

	if(xx1 <= view->x2 || xx2 >= view->x1) {
		if(xx1 < view->x1)
			xx1 = view->x1;
		if(xx2 > view->x2)
			xx2 = view->x2;

		if(color & MOIRE)
			draw_hline_moired(bitmap, xx1, xx2, y, color);
		else
			draw_hline(bitmap, xx1, xx2, y, color);
	}
}

void model1_state::fill_quad(bitmap_rgb32 &bitmap, view_t *view, const quad_t& q) const
{
	spoint_t p[8];
	int color = q.col;

	if (color < 0)
	{
		color = -1 - color;
		LOG_TGP_DEV(this,("VIDEOD: Q (%d, %d)-(%d, %d)-(%d, %d)-(%d, %d)\n",
					q.p[0]->s.x, q.p[0]->s.y,
					q.p[1]->s.x, q.p[1]->s.y,
					q.p[2]->s.x, q.p[2]->s.y,
					q.p[3]->s.x, q.p[3]->s.y));
	}

	for (int i = 0; i < 4; i++)
	{
		p[i].x = p[i+4].x = q.p[i]->s.x << FRAC_SHIFT;
		p[i].y = p[i+4].y = q.p[i]->s.y;
	}

	int pmin = 0;
	int pmax = 0;
	for (int i = 1; i < 4; i++)
	{
		if(p[i].y < p[pmin].y)
		{
			pmin = i;
		}
		if(p[i].y > p[pmax].y)
		{
			pmax = i;
		}
	}

	int32_t cury = p[pmin].y;
	int32_t limy = p[pmax].y;

	int32_t x1, x2;
	if (cury == limy)
	{
		x1 = p[0].x;
		x2 = p[0].x;
		for (int i = 1; i < 4; i++)
		{
			if (p[i].x < x1)
			{
				x1 = p[i].x;
			}
			if (p[i].x > x2)
			{
				x2 = p[i].x;
			}
		}
		fill_line(bitmap, view, color, cury, x1, x2);
		return;
	}

	if(cury > view->y2)
	{
		return;
	}
	if(limy <= view->y1)
	{
		return;
	}

	if(limy > view->y2)
	{
		limy = view->y2;
	}

	int ps1 = pmin+4;
	int ps2 = pmin;

	goto startup;

	int32_t sl1, sl2;
	for(;;)
	{
		if (p[ps1 - 1].y == p[ps2 + 1].y)
		{
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if(cury >= limy)
			{
				break;
			}
			ps1--;
			ps2++;

		startup:
			while(p[ps1-1].y == cury)
			{
				ps1--;
			}
			while(p[ps2+1].y == cury)
			{
				ps2++;
			}
			x1 = p[ps1].x;
			x2 = p[ps2].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
		else if (p[ps1 - 1].y < p[ps2 + 1].y)
		{
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps1 - 1].y, &x1, &x2);
			cury = p[ps1 - 1].y;
			if(cury >= limy)
			{
				break;
			}
			ps1--;
			while(p[ps1 - 1].y == cury)
			{
				ps1--;
			}
			x1 = p[ps1].x;
			sl1 = (x1 - p[ps1 - 1].x) / (cury - p[ps1 - 1].y);
		}
		else
		{
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps2 + 1].y, &x1, &x2);
			cury = p[ps2 + 1].y;
			if(cury >= limy)
			{
				break;
			}
			ps2++;
			while(p[ps2 + 1].y == cury)
			{
				ps2++;
			}
			x2 = p[ps2].x;
			sl2 = (x2 - p[ps2 + 1].x) / (cury - p[ps2 + 1].y);
		}
	}
	if(cury == limy)
	{
		fill_line(bitmap, view, color, cury, x1, x2);
	}
}

#if 0
void model1_state::draw_line(bitmap_rgb32 &bitmap, model1_state::view_t *view, int color, int x1, int y1, int x2, int y2) const
{
	if ((x1 < view->x1 && x2 < view->x1) ||
		(x1 > view->x2 && x2 > view->x2) ||
		(y1 < view->y1 && y2 < view->y1) ||
		(y1 > view->y2 && y2 > view->y2))
		return;

	int x = x1;
	int y = y1;
	int s1x = 1;
	int s1y = 0;
	int s2x = 0;
	int s2y = 1;

	int d1 = x2-x1;
	int d2 = y2-y1;
	if (d1 < 0)
	{
		s1x = -s1x;
		d1 = -d1;
	}
	if (d2 < 0)
	{
		s2y = -s2y;
		d2 = -d2;
	}
	if (d1 < d2)
	{
		int t = s1x;
		s1x = s2x;
		s2x = t;
		t = s1y;
		s1y = s2y;
		s2y = t;
		t = d1;
		d1 = d2;
		d2 = t;
	}

	if(d1 > 600)
	{
		return;
	}

	int cur = 0;
	while (x != x2 || y != y2)
	{
		if (x >= view->x1 && x <= view->x2 && y >= view->y1 && y <= view->y2)
		{
			bitmap.pix32(y, x) = color;
		}
		x += s1x;
		y += s1y;
		cur += d2;
		if (cur >= d1)
		{
			cur -= d1;
			x += s2x;
			y += s2y;
		}
	}
	if (x >= view->x1 && x <= view->x2 && y >= view->y1 && y <= view->y2)
	{
		bitmap.pix16(y, x) = color;
	}
}
#endif

static int comp_quads(const void *q1, const void *q2)
{
	model1_state::quad_t* const* c1 = static_cast<model1_state::quad_t* const*>(q1);
	model1_state::quad_t* const* c2 = static_cast<model1_state::quad_t* const*>(q2);

	return (*c1)->compare(*c2);
}

int model1_state::quad_t::compare(const model1_state::quad_t* other) const
{
	float z2 = other->z;

	if (z < z2)
		return +1;
	if (z > z2)
		return -1;

	if (this - other < 0)
		return -1;

	return +1;
}

void model1_state::sort_quads() const
{
	const int count = m_quadpt - m_quaddb;
	for (int i = 0; i < count; i++)
	{
		m_quadind[i] = m_quaddb + i;
	}
	qsort(m_quadind, count, sizeof(model1_state::quad_t*), comp_quads);
}

void model1_state::unsort_quads() const
{
	const int count = m_quadpt - m_quaddb;
	for (int i = 0; i < count; i++)
	{
		m_quadind[i] = m_quaddb + i;
	}
}


void model1_state::draw_quads(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	view_t *view = m_view.get();
	int count = m_quadpt - m_quaddb;

	/* clip to the cliprect */
	int save_x1 = view->x1;
	int save_x2 = view->x2;
	int save_y1 = view->y1;
	int save_y2 = view->y2;
	view->x1 = std::max(view->x1, cliprect.min_x);
	view->x2 = std::min(view->x2, cliprect.max_x);
	view->y1 = std::max(view->y1, cliprect.min_y);
	view->y2 = std::min(view->y2, cliprect.max_y);

	for (int i = 0; i < count; i++)
	{
		fill_quad(bitmap, view, *m_quadind[i]);
#if 0
		quad_t *q = m_quadind[i];
		draw_line(bitmap, m_palette->black_pen(), q.p[0]->s.x, q.p[0]->s.y, q.p[1]->s.x, q.p[1]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q.p[1]->s.x, q.p[1]->s.y, q.p[2]->s.x, q.p[2]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q.p[2]->s.x, q.p[2]->s.y, q.p[3]->s.x, q.p[3]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q.p[3]->s.x, q.p[3]->s.y, q.p[0]->s.x, q.p[0]->s.y);
#endif
	}

	view->x1 = save_x1;
	view->x2 = save_x2;
	view->y1 = save_y1;
	view->y2 = save_y2;
}
#if 0
uint16_t model1_state::scale_color(uint16_t color, float level) const
{
	int r = ((color >> 10) & 31) * level;
	int g = ((color >>  5) & 31) * level;
	int b =         (color & 31) * level;
	return (r << 10) | (g << 5) | b;
}
#endif
// xe = xc + (x/z * zm + tx)
// xe < x1 => xc + (x/z * zm + tx) < x1
//         => x/z < (x1-xc-tx)/zm
//         => x < z*(x1-xc-tx)/zm

// ye = yc - (y/z * zm + ty)
// ye < y1 => yc - (y/z * zm + ty) < y1
//         => -y/z < (y1-yc+ty)/zm
//         => y > -z*(y1-yc+ty)/zm

// xf = zf*a
// xx = x1*t+x2(1-t); zz = z1*t+z2(1-t)
// => x1*t+x2(1-t) = z1*t*a+z2*(1-t)*a
// => t*(x1-x2+a*(z2-z1) = -x2+a*z2
// => t = (z2*a-x2)/((z2-z1)*a-(x2-x1))

void model1_state::view_t::recompute_frustum()
{
	a_left   = ( x1 - xc - viewx) / zoomx;
	a_right  = ( x2 - xc - viewx) / zoomx;
	a_bottom = (-y1 + yc - viewy) / zoomy;
	a_top    = (-y2 + yc - viewy) / zoomy;
}

bool model1_state::fclip_isc_bottom(view_t *view, point_t *p)
{
	return p->y > (p->z * view->a_bottom);
}

void model1_state::fclip_clip_bottom(view_t *view, point_t *pt, point_t *p1, point_t *p2)
{
	float t = (p2->z * view->a_bottom-p2->y) / ((p2->z - p1->z) * view->a_bottom - (p2->y - p1->y));
	pt->x = p1->x * t + p2->x * (1 - t);
	pt->y = p1->y * t + p2->y * (1 - t);
	pt->z = p1->z * t + p2->z * (1 - t);
	view->project_point(pt);
}

bool model1_state::fclip_isc_top(view_t *view, point_t *p)
{
	return p->y < (p->z * view->a_top);
}

void model1_state::fclip_clip_top(view_t *view, point_t *pt, point_t *p1, point_t *p2)
{
	float t = (p2->z * view->a_top - p2->y) / ((p2->z - p1->z) * view->a_top - (p2->y - p1->y));
	pt->x = p1->x * t + p2->x * (1 - t);
	pt->y = p1->y * t + p2->y * (1 - t);
	pt->z = p1->z * t + p2->z * (1 - t);
	view->project_point(pt);
}

bool model1_state::fclip_isc_left(view_t *view, point_t *p)
{
	return p->x < (p->z * view->a_left);
}

void model1_state::fclip_clip_left(view_t *view, point_t *pt, point_t *p1, point_t *p2)
{
	float t = (p2->z * view->a_left - p2->x) / ((p2->z - p1->z) * view->a_left - (p2->x - p1->x));
	pt->x = p1->x * t + p2->x * (1 - t);
	pt->y = p1->y * t + p2->y * (1 - t);
	pt->z = p1->z * t + p2->z * (1 - t);
	view->project_point(pt);
}

bool model1_state::fclip_isc_right(view_t *view, point_t *p)
{
	return p->x > (p->z * view->a_right);
}

void model1_state::fclip_clip_right(view_t *view, point_t *pt, point_t *p1, point_t *p2)
{
	float t = (p2->z * view->a_right - p2->x) / ((p2->z - p1->z) * view->a_right - (p2->x - p1->x));
	pt->x = p1->x * t + p2->x * (1 - t);
	pt->y = p1->y * t + p2->y * (1 - t);
	pt->z = p1->z * t + p2->z * (1 - t);
	view->project_point(pt);
}

void model1_state::fclip_push_quad_next(int level, quad_t& q, point_t *p1, point_t *p2, point_t *p3, point_t *p4)
{
	quad_t cquad(q.col, q.z, p1, p2, p3, p4);
	fclip_push_quad(level+1, cquad);
}

void model1_state::fclip_push_quad(int level, quad_t& q)
{
	view_t *view = m_view.get();

	if (level == 4)
	{
		LOG_TGP(("VIDEOCQ %d", level));
		for (int i = 0; i < 4; i++)
			LOG_TGP((" (%f, %f, %f)", q.p[i]->x, q.p[i]->y, q.p[i]->z));
		LOG_TGP(("\n"));
		*m_quadpt = q;
		m_quadpt++;
		return;
	}

	bool is_out[4];
	for (int i = 0; i < 4; i++)
	{
		is_out[i] = m_clipfn[level].m_isclipped(view, q.p[i]);
	}

	LOG_TGP(("VIDEOCQ %d", level));
	for (int i = 0; i < 4; i++)
		LOG_TGP((" (%f, %f, %f, %d)", q.p[i]->x, q.p[i]->y, q.p[i]->z, is_out[i]));
	LOG_TGP(("\n"));

	// No clipping
	if(!is_out[0] && !is_out[1] && !is_out[2] && !is_out[3])
	{
		fclip_push_quad(level+1, q);
		return;
	}

	// Full clipping
	if(is_out[0] && is_out[1] && is_out[2] && is_out[3])
		return;

	// Find n so that point n is clipped and n-1 isn't
	int i;
	for (i = 0; i < 4; i++)
		if(is_out[i] && !is_out[(i-1)&3])
			break;

	point_t* pt[4];
	bool is_out2[4];
	for (int j = 0; j < 4; j++)
	{
		pt[j] = q.p[(i + j) & 3];
		is_out2[j] = is_out[(i + j) & 3];
	}

	// At this point, pt[0] is clipped out and pt[3] isn't.  Test for the 4 possible cases
	if (is_out2[1])
	{
		if (is_out2[2])
		{
			// pt 0,1,2 clipped out, one triangle left
			m_clipfn[level].m_clip(view, m_pointpt, pt[2], pt[3]);
			point_t* pi1 = m_pointpt++;
			m_clipfn[level].m_clip(view, m_pointpt, pt[3], pt[0]);
			point_t* pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[3], pi2, pi2);
		}
		else
		{
			// pt 0,1 clipped out, one quad left
			m_clipfn[level].m_clip(view, m_pointpt, pt[1], pt[2]);
			point_t* pi1 = m_pointpt++;
			m_clipfn[level].m_clip(view, m_pointpt, pt[3], pt[0]);
			point_t* pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[2], pt[3], pi2);
		}
	}
	else
	{
		if (is_out2[2])
		{
			// pt 0,2 clipped out, shouldn't happen, two triangles
			m_clipfn[level].m_clip(view, m_pointpt, pt[0], pt[1]);
			point_t* pi1 = m_pointpt++;
			m_clipfn[level].m_clip(view, m_pointpt, pt[1], pt[2]);
			point_t* pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[1], pi2, pi2);
			m_clipfn[level].m_clip(view, m_pointpt, pt[2], pt[3]);
			pi1 = m_pointpt++;
			m_clipfn[level].m_clip(view, m_pointpt, pt[3], pt[0]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[3], pi2, pi2);
		}
		else
		{
			// pt 0 clipped out, one decagon left, split in quad+tri
			m_clipfn[level].m_clip(view, m_pointpt, pt[0], pt[1]);
			point_t* pi1 = m_pointpt++;
			m_clipfn[level].m_clip(view, m_pointpt, pt[3], pt[0]);
			point_t* pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[1], pt[2], pt[3]);
			fclip_push_quad_next(level, q, pt[3], pi2, pi1, pi1);
		}
	}
}

float model1_state::min4f(float a, float b, float c, float d)
{
	float m = a;
	if(b<m)
		m = b;
	if(c<m)
		m = c;
	if(d<m)
		m = d;
	return m;
}

float model1_state::max4f(float a, float b, float c, float d)
{
	float m = a;
	if (b > m)
		m = b;
	if (c > m)
		m = c;
	if (d > m)
		m = d;
	return m;
}

#ifdef UNUSED_DEFINITION
static const uint8_t num_of_times[]={1,1,1,1,2,2,2,3};
#endif
float model1_state::compute_specular(glm::vec3& normal, glm::vec3& light, float diffuse, int lmode)
{
#if 0
	int p = m_view->lightparams[lmode].p & 7;
	float sv = m_view->lightparams[lmode].s;

	//This is how it should be according to model2 geo program, but doesn't work fine
	float s = 2 * (diffuse * normal.z - light.z);
	for (int i = 0; i < num_of_times[p]; i++)
	{
		s *= s;
	}
	s *= sv;
	if (s < 0.0f)
	{
		return 0.0f;
	}
	if (s > 1.0f)
	{
		return 1.0f;
	}
	return s;

	// ???
	//return fabs(diffuse)*sv;
#endif

	return 0;
}

void model1_state::push_object(uint32_t tex_adr, uint32_t poly_adr, uint32_t size) {
#if 0
	int dump;
#endif

	float *poly_data;
	if (poly_adr & 0x800000)
		poly_data = (float *)m_poly_ram.get();
	else
		poly_data = (float *)m_poly_rom.target();

	poly_adr &= 0x7fffff;
#if 0
	dump = poly_adr == 0x944ea;
	dump = 0;
#endif

#if 0
	if (poly_adr < 0x10000 || poly_adr >= 0x80000)
		return;

	if (poly_adr < 0x40000 || poly_adr >= 0x50000)
		return;

	if (poly_adr == 0x4c7db || poly_adr == 0x4cdaa || poly_adr == 0x4d3e7)
		return;

	if (poly_adr != 0x483e5)
		return;
#endif

	if (true) {
		LOG_TGP(("XVIDEO:   draw object (%x, %x, %x)\n", tex_adr, poly_adr, size));
	}

	if (!size)
		size = 0xffffffff;

	point_t *old_p0 = m_pointpt++;
	point_t *old_p1 = m_pointpt++;

	old_p0->x = poly_data[poly_adr + 0];
	old_p0->y = poly_data[poly_adr + 1];
	old_p0->z = poly_data[poly_adr + 2];
	old_p1->x = poly_data[poly_adr + 3];
	old_p1->y = poly_data[poly_adr + 4];
	old_p1->z = poly_data[poly_adr + 5];
	m_view->transform_point(old_p0);
	m_view->transform_point(old_p1);

	if (old_p0->z > 0)
	{
		m_view->project_point(old_p0);
	}
	else
	{
		old_p0->s.x = old_p0->s.y = 0;
	}

	if (old_p1->z > 0)
	{
		m_view->project_point(old_p1);
	}
	else
	{
		old_p1->s.x = old_p1->s.y = 0;
	}

	float old_z = 0;

	poly_adr += 6;

	for (int i = 0; i < size; i++)
	{
#if 0
		LOG_TGP(("VIDEO:     %08x (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)\n",
			*(uint32_t *)(poly_data + poly_adr) & ~(0x01800303),
			poly_data[poly_adr + 1], poly_data[poly_adr + 2], poly_data[poly_adr + 3],
			poly_data[poly_adr + 4], poly_data[poly_adr + 5], poly_data[poly_adr + 6],
			poly_data[poly_adr + 7], poly_data[poly_adr + 8], poly_data[poly_adr + 9]));
#endif
		uint32_t flags = *reinterpret_cast<uint32_t*>(poly_data + poly_adr);

		int type = flags & 3;
		if (!type)
			break;

		if (flags & 0x00001000)
			tex_adr++;
		int lightmode = (flags >> 17) & 15;

		point_t *p0 = m_pointpt++;
		point_t *p1 = m_pointpt++;

		glm::vec3 vn(poly_data[poly_adr + 1], poly_data[poly_adr + 2], poly_data[poly_adr + 3]);
		p0->x = poly_data[poly_adr + 4];
		p0->y = poly_data[poly_adr + 5];
		p0->z = poly_data[poly_adr + 6];
		p1->x = poly_data[poly_adr + 7];
		p1->y = poly_data[poly_adr + 8];
		p1->z = poly_data[poly_adr + 9];

		int link = (flags >> 8) & 3;

		m_view->transform_vector(vn);

		m_view->transform_point(p0);
		m_view->transform_point(p1);
		if (p0->z > 0)
		{
			m_view->project_point(p0);
		}
		else
		{
			p0->s.x = p0->s.y = 0;
		}

		if (p1->z > 0)
		{
			m_view->project_point(p1);
		}
		else
		{
			p1->s.x = p1->s.y = 0;
		}

#if 0
		if (dump)
			LOG_TGP(("VIDEO:     %08x (%f, %f, %f) (%f, %f, %f)\n",
				*(uint32_t *)(poly_data + poly_adr),
				p0->x, p0->y, p0->z,
				p1->x, p1->y, p1->z));
#endif


#if 0
		if (true || dump) {
			LOG_TGP(("VIDEO:     %08x (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n",
				*(uint32_t *)(poly_data + poly_adr),
				old_p0->s.x, old_p0->s.y,
				old_p1->s.x, old_p1->s.y,
				p0->s.x, p0->s.y,
				p1->s.x, p1->s.y));
		}
#endif

		quad_t cquad;

		if (!link)
			goto next;

		if (!(flags & 0x00004000) && view_determinant(old_p1, old_p0, p0) > 0)
			goto next;

		vn = glm::normalize(vn);

		cquad.p[0] = old_p1;
		cquad.p[1] = old_p0;
		cquad.p[2] = p0;
		cquad.p[3] = p1;

		switch ((flags >> 10) & 3)
		{
			case 0:
				cquad.z = old_z;
				break;
			case 1:
				cquad.z = old_z = min4f(old_p1->z, old_p0->z, p0->z, p1->z);
				break;
			case 2:
				cquad.z = old_z = max4f(old_p1->z, old_p0->z, p0->z, p1->z);
				break;
			case 3:
			default:
				cquad.z = 0.0;
				break;
		}

		{
#if 0
			float dif = mult_vector(&vn, &view->light);
			float ln = view->lightparams[lightmode].a + view->lightparams[lightmode].d*MAX(0.0, dif);
			cquad.col = scale_color(machine().pens[0x1000 | (m_tgp_ram[tex_adr - 0x40000] & 0x3ff)], MIN(1.0, ln));
			cquad.col = scale_color(machine().pens[0x1000 | (m_tgp_ram[tex_adr - 0x40000] & 0x3ff)], MIN(1.0, ln));
#endif

			float dif = glm::dot(vn, m_view->light);
			float spec = compute_specular(vn, m_view->light, dif, lightmode);
			float ln = m_view->lightparams[lightmode].a + m_view->lightparams[lightmode].d * std::max(0.0f, dif) + spec;
			int lumval = 255.0f * std::min(1.0f, ln);
			int color = m_paletteram16[0x1000 | (m_tgp_ram[tex_adr - 0x40000] & 0x3ff)];
			int r = (color >> 0x0) & 0x1f;
			int g = (color >> 0x5) & 0x1f;
			int b = (color >> 0xA) & 0x1f;

			lumval >>= 2; //there must be a luma translation table somewhere
			if (lumval > 0x3f)
				lumval = 0x3f;
			else if (lumval < 0)
				lumval = 0;

			r = (m_color_xlat[(r << 8) | lumval | 0x0] >> 3) & 0x1f;
			g = (m_color_xlat[(g << 8) | lumval | 0x2000] >> 3) & 0x1f;
			b = (m_color_xlat[(b << 8) | lumval | 0x4000] >> 3) & 0x1f;
			cquad.col = (pal5bit(r) << 16) | (pal5bit(g) << 8) | (pal5bit(b) << 0);
		}

		if (flags & 0x00002000)
			cquad.col |= MOIRE;

		fclip_push_quad(0, cquad);

	next:
		poly_adr += 10;
		switch (link) {
			case 0:
			case 2:
				old_p0 = p0;
				old_p1 = p1;
				break;
			case 1:
				old_p1 = p0;
				break;
			case 3:
				old_p0 = p1;
				break;
		}
	}
}


int model1_state::push_direct(int list_offset) {
	uint32_t tex_adr = readi(list_offset + 2);
	//  v1      = readi(list_offset+2+2);
	//  v2      = readi(list_offset+2+10);

	point_t *old_p0 = m_pointpt++;
	point_t *old_p1 = m_pointpt++;

	old_p0->x = readf(list_offset + 2 + 4);
	old_p0->y = readf(list_offset + 2 + 6);
	old_p0->z = readf(list_offset + 2 + 8);
	old_p1->x = readf(list_offset + 2 + 12);
	old_p1->y = readf(list_offset + 2 + 14);
	old_p1->z = readf(list_offset + 2 + 16);

	LOG_TGP(("VIDEOD start direct\n"));
	LOG_TGP(("VIDEOD (%f, %f, %f) (%f, %f, %f)\n",
		old_p0->x, old_p0->y, old_p0->z,
		old_p1->x, old_p1->y, old_p1->z));

	//m_view-transform_point(old_p0);
	//m_view->transform_point(old_p1);
	if (old_p0->z > 0)
	{
		m_view->project_point_direct(old_p0);
	}
	else
	{
		old_p0->s.x = old_p0->s.y = 0;
	}

	if (old_p1->z > 0)
	{
		m_view->project_point_direct(old_p1);
	}
	else
	{
		old_p1->s.x = old_p1->s.y = 0;
	}

	list_offset += 18;

	for (;;) {
		uint32_t flags = readi(list_offset + 2);

		int type = flags & 3;
		if (!type)
			break;

		if (flags & 0x00001000)
			tex_adr++;

		// list+2+2 is luminosity
		// list+2+4 is 0?
		// list+2+12 is z?

		point_t *p0 = m_pointpt++;
		point_t *p1 = m_pointpt++;

		uint32_t lum = readi(list_offset + 2 + 2);
		//      v1    = readi(list_offset+2+4);

		float z = 0;
		if (type == 2)
		{
			p0->x = readf(list_offset + 2 + 6);
			p0->y = readf(list_offset + 2 + 8);
			p0->z = readf(list_offset + 2 + 10);
			z = p0->z;
			LOG_TGP(("VIDEOD %08x %08x (%f, %f, %f)\n",
				flags, lum,
				p0->x, p0->y, p0->z));
			*p1 = *p0;
			list_offset += 12;
		}
		else
		{
			p0->x = readf(list_offset + 2 + 6);
			p0->y = readf(list_offset + 2 + 8);
			p0->z = readf(list_offset + 2 + 10);
			p1->x = readf(list_offset + 2 + 14);
			p1->y = readf(list_offset + 2 + 16);
			p1->z = readf(list_offset + 2 + 18);
			z = readf(list_offset + 2 + 12);
			LOG_TGP(("VIDEOD %08x %08x (%f, %f, %f) (%f, %f, %f)\n",
				flags, lum,
				p0->x, p0->y, p0->z,
				p1->x, p1->y, p1->z));
			list_offset += 20;
		}

		int link = (flags >> 8) & 3;

		//m_view->transform_point(p0);
		//m_view->transform_point(p1);
		if (p0->z > 0)
		{
			m_view->project_point_direct(p0);
		}
		if (p1->z > 0)
		{
			m_view->project_point_direct(p1);
		}

#if 1
		if (old_p0 && old_p1)
			LOG_TGP(("VIDEOD:     %08x (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n",
				flags,
				old_p0->s.x, old_p0->s.y,
				old_p1->s.x, old_p1->s.y,
				p0->s.x, p0->s.y,
				p1->s.x, p1->s.y));
		else
			LOG_TGP(("VIDEOD:     %08x (%d, %d) (%d, %d)\n",
				flags,
				p0->s.x, p0->s.y,
				p1->s.x, p1->s.y));

#endif

		quad_t cquad;
		if (!link)
			goto next;

		cquad.p[0] = old_p1;
		cquad.p[1] = old_p0;
		cquad.p[2] = p0;
		cquad.p[3] = p1;
		cquad.z = z;
		{
			int lumval = ((float)(lum >> 24)) * 2.0f;
			int color = m_paletteram16[0x1000 | (m_tgp_ram[tex_adr - 0x40000] & 0x3ff)];
			int r = (color >> 0x0) & 0x1f;
			int g = (color >> 0x5) & 0x1f;
			int b = (color >> 0xA) & 0x1f;
			lumval >>= 2; //there must be a luma translation table somewhere
			if (lumval > 0x3f) lumval = 0x3f;
			else if (lumval < 0) lumval = 0;
			r = (m_color_xlat[(r << 8) | lumval | 0x0] >> 3) & 0x1f;
			g = (m_color_xlat[(g << 8) | lumval | 0x2000] >> 3) & 0x1f;
			b = (m_color_xlat[(b << 8) | lumval | 0x4000] >> 3) & 0x1f;
			cquad.col = (pal5bit(r) << 16) | (pal5bit(g) << 8) | (pal5bit(b) << 0);
		}
		//cquad.col  = scale_color(machine().pens[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)],((float) (lum>>24)) / 128.0);
		if (flags & 0x00002000)
			cquad.col |= MOIRE;

		fclip_push_quad(0, cquad);

	next:
		switch (link) {
		case 0:
		case 2:
			old_p0 = p0;
			old_p1 = p1;
			break;
		case 1:
			old_p1 = p0;
			break;
		case 3:
			old_p0 = p1;
			break;
		}
	}
	list_offset += 4;
	return list_offset;
}

int model1_state::skip_direct(int list_offset) const
{
	list_offset += 18;

	while (true) {
		uint32_t flags = readi(list_offset + 2);

		int type = flags & 3;
		if (!type)
			break;

		if (type == 2)
			list_offset += 12;
		else
			list_offset += 20;
	}
	list_offset += 4;
	return list_offset;
}

void model1_state::draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_quadpt != m_quaddb)
	{
		LOG_TGP(("VIDEO: sort&draw\n"));
		sort_quads();
		draw_quads(bitmap, cliprect);
	}

	m_quadpt = m_quaddb;
	m_pointpt = m_pointdb;
}


int model1_state::draw_direct(bitmap_rgb32 &bitmap, const rectangle &cliprect, int list_offset)
{
	LOG_TGP(("VIDEO:   draw direct %x\n", readi(list_offset + 2)));

	draw_objects(bitmap, cliprect);

	list_offset = push_direct(list_offset);

	unsort_quads();
	draw_quads(bitmap, cliprect);

	m_quadpt = m_quaddb;
	m_pointpt = m_pointdb;

	return list_offset;
}


void model1_state::set_current_render_list()
{
	if(!(m_listctl[0] & 4))
		m_listctl[0] = (m_listctl[0] & ~0x40) | (m_listctl[0] & 8 ? 0x40 : 0);
	m_display_list_current = m_listctl[0] & 0x40 ? m_display_list1 : m_display_list0;
}

int model1_state::get_list_number()
{
	if(!(m_listctl[0] & 4))
		m_listctl[0] = (m_listctl[0] & ~0x40) | (m_listctl[0] & 8 ? 0x40 : 0);
	return m_listctl[0] & 0x40 ? 0 : 1;
}

void model1_state::end_frame()
{
	if((m_listctl[0] & 4) && (m_screen->frame_number() & 1))
		m_listctl[0] ^= 0x40;
}

READ16_MEMBER(model1_state::model1_listctl_r)
{
	if(!offset)
		return m_listctl[0] | 0x30;
	else
		return m_listctl[1];
}

WRITE16_MEMBER(model1_state::model1_listctl_w)
{
	COMBINE_DATA(m_listctl + offset);
	LOG_TGP(("VIDEO: control=%08x\n", (m_listctl[1] << 16) | m_listctl[0]));
}

void model1_state::view_t::init_translation_matrix()
{
	memset(translation, 0, sizeof(translation));
	translation[0] = 1.0;
	translation[4] = 1.0;
	translation[8] = 1.0;
}

void model1_state::view_t::set_viewport(float xcenter, float ycenter, float xl, float xr, float yb, float yt)
{
	xc = xcenter;
	yc = ycenter;
	x1 = xl;
	x2 = xr;
	y1 = yb;
	y2 = yt;

	recompute_frustum();
}

void model1_state::view_t::set_lightparam(int index, float diffuse, float ambient, float specular, int power)
{
	lightparams[index].d = diffuse;
	lightparams[index].a = ambient;
	lightparams[index].s = specular;
	lightparams[index].p = power;
}

void model1_state::view_t::set_zoom(float x, float y)
{
	zoomx = x;
	zoomy = y;

	recompute_frustum();
}

void model1_state::view_t::set_light_direction(float x, float y, float z)
{
	light = glm::normalize(glm::vec3(x, y, z));
}

void model1_state::view_t::set_translation_matrix(float* mat)
{
	for (int i = 0; i < 12; i++)
	{
		translation[i] = mat[i];
	}
}

void model1_state::view_t::set_view_translation(float x, float y)
{
	viewx = x;
	viewy = y;

	recompute_frustum();
}



void model1_state::tgp_render(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_render_done = 1;
	if ((m_listctl[1] & 0x1f) == 0x1f)
	{
		set_current_render_list();
		int zz = 0;
		LOG_TGP(("VIDEO: render list %d\n", get_list_number()));

		m_view->init_translation_matrix();

		int list_offset = 0;
		for (;;) {
			int type = readi(list_offset + 0);

			switch (type)
			{
			case 0:
				list_offset += 2;
				break;
			case 1:
			case 0x41:
				// 1 = plane 1
				// 2 = ??  draw object (413d3, 17c4c, e)
				// 3 = plane 2
				// 4 = ??  draw object (408a8, a479, 9)
				// 5 = decor
				// 6 = ??  draw object (57bd4, 387460, 2ad)

				if (true || zz >= 666)
					push_object(readi(list_offset + 2), readi(list_offset + 4), readi(list_offset + 6));
				list_offset += 8;
				break;
			case 2:
			{
				list_offset = draw_direct(bitmap, cliprect, list_offset);
				break;
			}
			case 3:
			{
				LOG_TGP(("VIDEO:   viewport (%d, %d, %d, %d, %d, %d, %d)\n",
					readi(list_offset + 2),
					readi16(list_offset + 4), readi16(list_offset + 6), readi16(list_offset + 8),
					readi16(list_offset + 10), readi16(list_offset + 12), readi16(list_offset + 14)));

				draw_objects(bitmap, cliprect);

				float xc = readi16(list_offset + 4);
				float yc = 383 - (readi16(list_offset + 6) - 39);
				float x1 = readi16(list_offset + 8);
				float y2 = 383 - (readi16(list_offset + 10) - 39);
				float x2 = readi16(list_offset + 12);
				float y1 = 383 - (readi16(list_offset + 14) - 39);

				m_view->set_viewport(xc, yc, x1, x2, y1, y2);

				list_offset += 16;
				break;
			}
			case 4:
			{
				int adr = readi(list_offset + 2);
				int len = readi(list_offset + 4) + 1;
				LOG_TGP(("ZVIDEO:   color write, adr=%x, len=%x\n", adr, len));
				for (int i = 0; i < len; i++)
				{
					m_tgp_ram[adr - 0x40000 + i] = readi16(list_offset + 6 + 2 * i);
				}
				list_offset += 6 + len * 2;
				break;
			}
			case 5:
			{
				int adr = readi(list_offset + 2);
				int len = readi(list_offset + 4);
				for (int i = 0; i < len; i++)
				{
					m_poly_ram[adr - 0x800000 + i] = readi(list_offset + 2 * i + 6);
				}
				list_offset += 6 + len * 2;
				break;
			}
			case 6:
			{
				int adr = readi(list_offset + 2);
				int len = readi(list_offset + 4);
				LOG_TGP(("VIDEO:   upload data, adr=%x, len=%x\n", adr, len));
				for (int i = 0; i < len; i++)
				{
					int v = readi(list_offset + 6 + i * 2);
					float diffuse = (float(v & 0xff)) / 255.0f;
					float ambient = (float((v >> 8) & 0xff)) / 255.0f;
					float specular = (float((v >> 16) & 0xff)) / 255.0f;
					int power = (v >> 24) & 0xff;
					m_view->set_lightparam(i + adr, diffuse, ambient, specular, power);
				}
				list_offset += 6 + len * 2;
				break;
			}
			case 7:
				LOG_TGP(("VIDEO:   code 7 (%d)\n", readi(list_offset + 2)));
				zz++;
				list_offset += 4;
				break;
			case 8:
				LOG_TGP(("VIDEO:   select mode %08x\n", readi(list_offset + 2)));
				list_offset += 4;
				break;
			case 9:
				LOG_TGP(("VIDEO:   zoom (%f, %f)\n", readf(list_offset + 2), readf(list_offset + 4)));
				m_view->set_zoom(readf(list_offset + 2) * 4, readf(list_offset + 4) * 4);
				list_offset += 6;
				break;
			case 0xa:
				LOG_TGP(("VIDEO:   light vector (%f, %f, %f)\n", readf(list_offset + 2), readf(list_offset + 4), readf(list_offset + 6)));
				m_view->set_light_direction(readf(list_offset + 2), readf(list_offset + 4), readf(list_offset + 6));
				list_offset += 8;
				break;
			case 0xb:
			{
				LOG_TGP(("VIDEO:   matrix (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
					readf(list_offset + 2), readf(list_offset + 4), readf(list_offset + 6),
					readf(list_offset + 8), readf(list_offset + 10), readf(list_offset + 12),
					readf(list_offset + 14), readf(list_offset + 16), readf(list_offset + 18),
					readf(list_offset + 20), readf(list_offset + 22), readf(list_offset + 24)));
				float mat[12];
				for (int i = 0; i < 12; i++)
				{
					mat[i] = readf(list_offset + 2 + 2 * i);
				}
				m_view->set_translation_matrix(mat);
				list_offset += 26;
				break;
			}
			case 0xc:
				LOG_TGP(("VIDEO:   trans (%f, %f)\n", readf(list_offset + 2), readf(list_offset + 4)));
				m_view->set_view_translation(readf(list_offset + 2), readf(list_offset + 4));
				list_offset += 6;
				break;
			case 0xf:
				//case -1:
				goto end;
			default:
				LOG_TGP(("VIDEO:   unknown type %d\n", type));
				goto end;
			}
		}
	end:
		draw_objects(bitmap, cliprect);
	}
}


void model1_state::tgp_scan()
{
#if 0
	if (machine().input().code_pressed_once(KEYCODE_F)) {
		FILE *fp;
		fp = fopen("tgp-ram.bin", "w+b");
		if (fp) {
			fwrite(m_tgp_ram, (0x100000 - 0x40000) * 2, 1, fp);
			fclose(fp);
		}
		exit(0);
	}
#endif
	if (!m_render_done && (m_listctl[1] & 0x1f) == 0x1f)
	{
		set_current_render_list();
		// Skip everything but the data uploads
		LOG_TGP(("VIDEO: scan list %d\n", get_list_number()));

		int list_offset = 0;
		for (;;)
		{
			int type = readi(list_offset + 0);
			switch (type) {
				case 0:
					list_offset += 2;
					break;
				case 1:
				case 0x41:
					list_offset += 8;
					break;
				case 2:
					list_offset = skip_direct(list_offset);

					break;
				case 3:
					list_offset += 16;
					break;
				case 4:
				{
					int adr = readi(list_offset + 2);
					int len = readi(list_offset + 4) + 1;
					LOG_TGP(("ZVIDEO:   scan color write, adr=%x, len=%x\n", adr, len));
					for (int i = 0; i<len; i++)
					{
						m_tgp_ram[adr - 0x40000 + i] = readi16(list_offset + 6 + 2 * i);
					}
					list_offset += 6 + len * 2;
					break;
				}
				case 5:
				{
					int adr = readi(list_offset + 2);
					int len = readi(list_offset + 4);
					for (int i = 0; i < len; i++)
					{
						m_poly_ram[adr - 0x800000 + i] = readi(list_offset + 2 * i + 6);
					}
					list_offset += 6 + len * 2;
					break;
				}
				case 6:
				{
					int adr = readi(list_offset + 2);
					int len = readi(list_offset + 4);
					//LOG_TGP(("VIDEO:   upload data, adr=%x, len=%x\n", adr, len));
					for (int i = 0; i<len; i++)
					{
						int v = readi(list_offset + 6 + i * 2);
						m_view->lightparams[i + adr].d = (float(v & 0xff)) / 255.0f;
						m_view->lightparams[i + adr].a = (float((v >> 8) & 0xff)) / 255.0f;
						m_view->lightparams[i + adr].s = (float((v >> 16) & 0xff)) / 255.0f;
						m_view->lightparams[i + adr].p = (v >> 24) & 0xff;
						//LOG_TGP(("         %02X\n",v));
					}
					list_offset += 6 + len * 2;
					break;
				}
				case 7:
					list_offset += 4;
					break;
				case 8:
					list_offset += 4;
					break;
				case 9:
					list_offset += 6;
					break;
				case 0xa:
					list_offset += 8;
					break;
				case 0xb:
					list_offset += 26;
					break;
				case 0xc:
					list_offset += 6;
					break;
				case 0xf:
				case -1:
					goto end;
				default:
					LOG_TGP(("VIDEO:   unknown type %d\n", type));
					goto end;
			}
		}
	end:
		;
	}
	m_render_done = 0;
}

void model1_state::video_start()
{
	m_view = std::make_unique<model1_state::view_t>();

	m_poly_ram = make_unique_clear<uint32_t[]>(0x400000);
	m_tgp_ram = make_unique_clear<uint16_t[]>(0x100000-0x40000);
	m_pointdb = auto_alloc_array_clear(machine(), model1_state::point_t, 1000000*2);
	m_quaddb  = auto_alloc_array_clear(machine(), model1_state::quad_t, 1000000);
	m_quadind = auto_alloc_array_clear(machine(), model1_state::quad_t *, 1000000);

	m_pointpt = m_pointdb;
	m_quadpt = m_quaddb;
	m_listctl[0] = m_listctl[1] = 0;

	m_clipfn[0].m_isclipped = &model1_state::fclip_isc_bottom;
	m_clipfn[0].m_clip = &model1_state::fclip_clip_bottom;
	m_clipfn[1].m_isclipped = &model1_state::fclip_isc_top;
	m_clipfn[1].m_clip = &model1_state::fclip_clip_top;
	m_clipfn[2].m_isclipped = &model1_state::fclip_isc_left;
	m_clipfn[2].m_clip = &model1_state::fclip_clip_left;
	m_clipfn[3].m_isclipped = &model1_state::fclip_isc_right;
	m_clipfn[3].m_clip = &model1_state::fclip_clip_right;

	save_pointer(NAME(m_tgp_ram), 0x100000-0x40000);
	save_pointer(NAME(m_poly_ram), 0x40000);
	save_item(NAME(m_listctl));
}

uint32_t model1_state::screen_update_model1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	model1_state::view_t *view = m_view.get();
#if 0
	{
		bool mod = false;
		double delta = 1.0;

		if(machine().input().code_pressed(KEYCODE_F))
		{
			mod = true;
			view->vxx -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_G))
		{
			mod = true;
			view->vxx += delta;
		}
		if(machine().input().code_pressed(KEYCODE_H))
		{
			mod = true;
			view->vyy -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_J))
		{
			mod = true;
			view->vyy += delta;
		}
		if(machine().input().code_pressed(KEYCODE_K))
		{
			mod = true;
			view->vzz -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_L))
		{
			mod = true;
			view->vzz += delta;
		}
		if(machine().input().code_pressed(KEYCODE_U))
		{
			mod = true;
			view->ayy -= 0.05;
		}
		if(machine().input().code_pressed(KEYCODE_I))
		{
			mod = true;
			view->ayy += 0.05;
		}
		if(mod)
		{
			popmessage("%g,%g,%g:%g", view->vxx, view->vyy, view->vzz, view->ayy);
		}
	}
#endif

	view->ayyc = cos(view->ayy);
	view->ayys = sin(view->ayy);

	screen.priority().fill(0);
	bitmap.fill(m_palette->pen(0), cliprect);

	m_tiles->draw(screen, bitmap, cliprect, 6, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 4, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 2, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 0, 0, 0);

	tgp_render(bitmap, cliprect);

	m_tiles->draw(screen, bitmap, cliprect, 7, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 5, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 3, 0, 0);
	m_tiles->draw(screen, bitmap, cliprect, 1, 0, 0);

	return 0;
}

WRITE_LINE_MEMBER(model1_state::screen_vblank_model1)
{
	// on rising edge
	if (state)
	{
		tgp_scan();
		end_frame();
		LOG_TGP(("TGP: vsync\n"));
	}
}
