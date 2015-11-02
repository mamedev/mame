// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "video/segaic24.h"
#include "cpu/mb86233/mb86233.h"
#include "includes/model1.h"

#define LOG_TGP_VIDEO 0

#define LOG_TGP(x)  do { if (LOG_TGP_VIDEO) logerror x; } while (0)
#define LOG_TGP_DEV(d,x)  do { if (LOG_TGP_VIDEO) d->logerror x; } while (0)


// Model 1 geometrizer TGP and rasterizer simulation
enum { FRAC_SHIFT = 16 };
enum { MOIRE = 0x01000000 };



struct vector_t {
	float x, y, z;
};

struct lightparam {
	float a;
	float d;
	float s;
	int p;
};

struct view {
	int xc, yc, x1, y1, x2, y2;
	float zoomx, zoomy, transx, transy;
	float a_bottom, a_top, a_left, a_right;
	float vxx, vyy, vzz, ayy, ayyc, ayys;
	float trans_mat[12];
	struct vector_t light;
	struct lightparam lightparams[32];
};

struct m1_spoint {
	INT32 x, y;
};

struct m1_point {
	float x, y, z;
	float xx, yy;
	struct m1_spoint s;
};


struct quad_m1 {
	struct m1_point *p[4];
	float z;
	int col;
};



static UINT32 readi(const UINT16 *adr)
{
	return adr[0]|(adr[1] << 16);
}

static INT16 readi16(const UINT16 *adr)
{
	return adr[0];
}

static float readf(const UINT16 *adr)
{
	return u2f(readi(adr));
}

static void _transform_point(struct view *view, struct m1_point *p)
{
	struct m1_point q = *p;
	float *trans = view->trans_mat;
	float xx, zz;
	xx = trans[0]*q.x+trans[3]*q.y+trans[6]*q.z+trans[9]+view->vxx;
	p->y = trans[1]*q.x+trans[4]*q.y+trans[7]*q.z+trans[10]+view->vyy;
	zz = trans[2]*q.x+trans[5]*q.y+trans[8]*q.z+trans[11]+view->vzz;
	p->x = view->ayyc*xx-view->ayys*zz;
	p->z = view->ayys*xx+view->ayyc*zz;
}

static void transform_vector(struct view *view, struct vector_t *p)
{
	struct vector_t q = *p;
	float *trans = view->trans_mat;
	p->x = trans[0]*q.x+trans[3]*q.y+trans[6]*q.z;
	p->y = trans[1]*q.x+trans[4]*q.y+trans[7]*q.z;
	p->z = trans[2]*q.x+trans[5]*q.y+trans[8]*q.z;
}

static void normalize_vector(struct vector_t *p)
{
	float norm = sqrt(p->x*p->x+p->y*p->y+p->z*p->z);
	if(norm) {
		p->x /= norm;
		p->y /= norm;
		p->z /= norm;
	}
}

static float mult_vector(const struct vector_t *p, const struct vector_t *q)
{
	return p->x*q->x+p->y*q->y+p->z*q->z;
}

static float view_determinant(const struct m1_point *p1, const struct m1_point *p2, const struct m1_point *p3)
{
	float x1 = p2->x - p1->x;
	float y1 = p2->y - p1->y;
	float z1 = p2->z - p1->z;
	float x2 = p3->x - p1->x;
	float y2 = p3->y - p1->y;
	float z2 = p3->z - p1->z;

	return p1->x*(y1*z2-y2*z1) + p1->y*(z1*x2-z2*x1) + p1->z*(x1*y2-x2*y1);
}


static void project_point(struct view *view, struct m1_point *p)
{
	p->xx = p->x / p->z;
	p->yy = p->y / p->z;
	p->s.x = view->xc+(p->xx*view->zoomx+view->transx);
	p->s.y = view->yc-(p->yy*view->zoomy+view->transy);
}

static void project_point_direct(struct view *view, struct m1_point *p)
{
	p->xx = p->x /*/ p->z*/;
	p->yy = p->y /*/ p->z*/;
	p->s.x = view->xc+(p->xx);
	p->s.y = view->yc-(p->yy);
}


static void draw_hline(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color)
{
	UINT32 *base = &bitmap.pix32(y);
	while(x1 <= x2) {
		base[x1] = color;
		x1++;
	}
}

static void draw_hline_moired(bitmap_rgb32 &bitmap, int x1, int x2, int y, int color)
{
	UINT32 *base = &bitmap.pix32(y);
	while(x1 <= x2) {
		if((x1^y)&1)
			base[x1] = color;
		x1++;
	}
}

static void fill_slope(bitmap_rgb32 &bitmap, struct view *view, int color, INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 y1, INT32 y2, INT32 *nx1, INT32 *nx2)
{
	if(y1 > view->y2)
		return;

	if(y2 <= view->y1) {
		int delta = y2-y1;
		*nx1 = x1+delta*sl1;
		*nx2 = x2+delta*sl2;
		return;
	}

	if(y2 > view->y2)
		y2 = view->y2+1;

	if(y1 < view->y1) {
		int delta = view->y1 - y1;
		x1 += delta*sl1;
		x2 += delta*sl2;
		y1 = view->y1;
	}

	if(x1 > x2 || (x1==x2 && sl1 > sl2)) {
		INT32 t, *tp;
		t = x1;
		x1 = x2;
		x2 = t;
		t = sl1;
		sl1 = sl2;
		sl2 = t;
		tp = nx1;
		nx1 = nx2;
		nx2 = tp;
	}

	while(y1 < y2) {
		if(y1 >= view->y1) {
			int xx1 = x1>>FRAC_SHIFT;
			int xx2 = x2>>FRAC_SHIFT;
			if(xx1 <= view->x2 || xx2 >= view->x1) {
				if(xx1 < view->x1)
					xx1 = view->x1;
				if(xx2 > view->x2)
					xx2 = view->x2;

				if(color & MOIRE)
					draw_hline_moired(bitmap, xx1, xx2, y1, color);
				else
					draw_hline(bitmap, xx1, xx2, y1, color);
			}
		}

		x1 += sl1;
		x2 += sl2;
		y1++;
	}
	*nx1 = x1;
	*nx2 = x2;
}

static void fill_line(bitmap_rgb32 &bitmap, struct view *view, int color, INT32 y, INT32 x1, INT32 x2)
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

static void fill_quad(device_t *device,bitmap_rgb32 &bitmap, struct view *view, const struct quad_m1 *q)
{
	INT32 sl1, sl2, cury, limy, x1, x2;
	int pmin, pmax, i, ps1, ps2;
	struct m1_spoint p[8];
	int color = q->col;

	if(color < 0) {
		color = -1-color;
		LOG_TGP_DEV(device,("VIDEOD: Q (%d, %d)-(%d, %d)-(%d, %d)-(%d, %d)\n",
					q->p[0]->s.x, q->p[0]->s.y,
					q->p[1]->s.x, q->p[1]->s.y,
					q->p[2]->s.x, q->p[2]->s.y,
					q->p[3]->s.x, q->p[3]->s.y));
	}

	for(i=0; i<4; i++) {
		p[i].x = p[i+4].x = q->p[i]->s.x << FRAC_SHIFT;
		p[i].y = p[i+4].y = q->p[i]->s.y;
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
		for(i=1; i<4; i++) {
			if(p[i].x < x1)
				x1 = p[i].x;
			if(p[i].x > x2)
				x2 = p[i].x;
		}
		fill_line(bitmap, view, color, cury, x1, x2);
		return;
	}

	if(cury > view->y2)
		return;
	if(limy <= view->y1)
		return;

	if(limy > view->y2)
		limy = view->y2;

	ps1 = pmin+4;
	ps2 = pmin;

	goto startup;

	for(;;) {
		if(p[ps1-1].y == p[ps2+1].y) {
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps1-1].y, &x1, &x2);
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
			x2 = p[ps2].x;
			sl1 = (x1-p[ps1-1].x)/(cury-p[ps1-1].y);
			sl2 = (x2-p[ps2+1].x)/(cury-p[ps2+1].y);
		} else if(p[ps1-1].y < p[ps2+1].y) {
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps1-1].y, &x1, &x2);
			cury = p[ps1-1].y;
			if(cury >= limy)
				break;
			ps1--;
			while(p[ps1-1].y == cury)
				ps1--;
			x1 = p[ps1].x;
			sl1 = (x1-p[ps1-1].x)/(cury-p[ps1-1].y);
		} else {
			fill_slope(bitmap, view, color, x1, x2, sl1, sl2, cury, p[ps2+1].y, &x1, &x2);
			cury = p[ps2+1].y;
			if(cury >= limy)
				break;
			ps2++;
			while(p[ps2+1].y == cury)
				ps2++;
			x2 = p[ps2].x;
			sl2 = (x2-p[ps2+1].x)/(cury-p[ps2+1].y);
		}
	}
	if(cury == limy)
		fill_line(bitmap, view, color, cury, x1, x2);
}
#if 0
static void draw_line(bitmap_rgb32 &bitmap, struct view *view, int color, int x1, int y1, int x2, int y2)
{
	int s1x, s1y, s2x, s2y;
	int d1, d2;
	int cur;
	int x, y;

	if((x1 < view->x1 && x2 < view->x1) ||
		(x1 > view->x2 && x2 > view->x2) ||
		(y1 < view->y1 && y2 < view->y1) ||
		(y1 > view->y2 && y2 > view->y2))
		return;

	x = x1;
	y = y1;
	s1x = 1;
	s1y = 0;
	s2x = 0;
	s2y = 1;

	d1 = x2-x1;
	d2 = y2-y1;
	if(d1<0) {
		s1x = -s1x;
		d1 = -d1;
	}
	if(d2<0) {
		s2y = -s2y;
		d2 = -d2;
	}
	if(d1 < d2) {
		int t;
		t = s1x;
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
		return;

	cur = 0;
	while(x != x2 || y != y2) {
		if(x>=view->x1 && x<=view->x2 && y>=view->y1 && y<=view->y2)
			bitmap.pix32(y, x) = color;
		x += s1x;
		y += s1y;
		cur += d2;
		if(cur >= d1) {
			cur -= d1;
			x += s2x;
			y += s2y;
		}
	}
	if(x>=view->x1 && x<=view->x2 && y>=view->y1 && y<=view->y2)
		bitmap.pix16(y, x) = color;
}
#endif
static int comp_quads(const void *q1, const void *q2)
{
	float z1 = (*(const struct quad_m1 **)q1)->z;
	float z2 = (*(const struct quad_m1 **)q2)->z;

	if(z1<z2)
		return +1;
	if(z1>z2)
		return -1;

	if (*(const struct quad_m1 **)q1 - *(const struct quad_m1 **)q2 < 0)
		return -1;

	return +1;
}

void model1_state::sort_quads()
{
	int count = m_quadpt - m_quaddb;
	int i;
	for(i=0; i<count; i++)
		m_quadind[i] = m_quaddb+i;
	qsort(m_quadind, count, sizeof(struct quad_m1 *), comp_quads);
}

void model1_state::unsort_quads()
{
	int count = m_quadpt - m_quaddb;
	int i;
	for(i=0; i<count; i++)
		m_quadind[i] = m_quaddb+i;
}


void model1_state::draw_quads(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	struct view *view = m_view;
	int count = m_quadpt - m_quaddb;
	int i;

	/* clip to the cliprect */
	int save_x1 = view->x1;
	int save_x2 = view->x2;
	int save_y1 = view->y1;
	int save_y2 = view->y2;
	view->x1 = MAX(view->x1, cliprect.min_x);
	view->x2 = MIN(view->x2, cliprect.max_x);
	view->y1 = MAX(view->y1, cliprect.min_y);
	view->y2 = MIN(view->y2, cliprect.max_y);

	for(i=0; i<count; i++) {
		struct quad_m1 *q = m_quadind[i];

		fill_quad(this, bitmap, view, q);
#if 0
		draw_line(bitmap, m_palette->black_pen(), q->p[0]->s.x, q->p[0]->s.y, q->p[1]->s.x, q->p[1]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q->p[1]->s.x, q->p[1]->s.y, q->p[2]->s.x, q->p[2]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q->p[2]->s.x, q->p[2]->s.y, q->p[3]->s.x, q->p[3]->s.y);
		draw_line(bitmap, m_palette->black_pen(), q->p[3]->s.x, q->p[3]->s.y, q->p[0]->s.x, q->p[0]->s.y);
#endif
	}

	view->x1 = save_x1;
	view->x2 = save_x2;
	view->y1 = save_y1;
	view->y2 = save_y2;
}
#if 0
static UINT16 scale_color(UINT16 color, float level)
{
	int r, g, b;
	r = ((color >> 10) & 31)*level;
	g = ((color >> 5) & 31)*level;
	b = (color & 31)*level;
	return (r<<10)|(g<<5)|b;
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

static void recompute_frustrum(struct view *view)
{
	view->a_left   = ( view->x1-view->xc-view->transx)/view->zoomx;
	view->a_right  = ( view->x2-view->xc-view->transx)/view->zoomx;
	view->a_bottom = (-view->y1+view->yc-view->transy)/view->zoomy;
	view->a_top    = (-view->y2+view->yc-view->transy)/view->zoomy;
}

static int fclip_isc_bottom(struct view *view, struct m1_point *p)
{
	return p->y > p->z*view->a_bottom;
}

static void fclip_clip_bottom(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2)
{
	float t = (p2->z*view->a_bottom-p2->y)/((p2->z-p1->z)*view->a_bottom-(p2->y-p1->y));
	pt->x = p1->x*t + p2->x*(1-t);
	pt->y = p1->y*t + p2->y*(1-t);
	pt->z = p1->z*t + p2->z*(1-t);
	project_point(view, pt);
}

static int fclip_isc_top(struct view *view, struct m1_point *p)
{
	return p->y < p->z*view->a_top;
}

static void fclip_clip_top(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2)
{
	float t = (p2->z*view->a_top-p2->y)/((p2->z-p1->z)*view->a_top-(p2->y-p1->y));
	pt->x = p1->x*t + p2->x*(1-t);
	pt->y = p1->y*t + p2->y*(1-t);
	pt->z = p1->z*t + p2->z*(1-t);
	project_point(view, pt);
}

static int fclip_isc_left(struct view *view, struct m1_point *p)
{
	return p->x < p->z*view->a_left;
}

static void fclip_clip_left(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2)
{
	float t = (p2->z*view->a_left-p2->x)/((p2->z-p1->z)*view->a_left-(p2->x-p1->x));
	pt->x = p1->x*t + p2->x*(1-t);
	pt->y = p1->y*t + p2->y*(1-t);
	pt->z = p1->z*t + p2->z*(1-t);
	project_point(view, pt);
}

static int fclip_isc_right(struct view *view, struct m1_point *p)
{
	return p->x > p->z*view->a_right;
}

static void fclip_clip_right(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2)
{
	float t = (p2->z*view->a_right-p2->x)/((p2->z-p1->z)*view->a_right-(p2->x-p1->x));
	pt->x = p1->x*t + p2->x*(1-t);
	pt->y = p1->y*t + p2->y*(1-t);
	pt->z = p1->z*t + p2->z*(1-t);
	project_point(view, pt);
}

static const struct {
	int (*isclipped)(struct view *view, struct m1_point *p);
	void (*clip)(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2);
} clipfn[4] = {
	{ fclip_isc_bottom, fclip_clip_bottom },
	{ fclip_isc_top,    fclip_clip_top },
	{ fclip_isc_left,   fclip_clip_left },
	{ fclip_isc_right,  fclip_clip_right },
};

void model1_state::fclip_push_quad_next(int level, struct quad_m1 *q,
									struct m1_point *p1, struct m1_point *p2, struct m1_point *p3, struct m1_point *p4)
{
	struct quad_m1 q2;
	q2.col = q->col;
	q2.z = q->z;
	q2.p[0] = p1;
	q2.p[1] = p2;
	q2.p[2] = p3;
	q2.p[3] = p4;

	fclip_push_quad(level+1, &q2);
}

void model1_state::fclip_push_quad(int level, struct quad_m1 *q)
{
	struct view *view = m_view;
	int i, j;
	struct m1_point *pt[4], *pi1, *pi2;
	int is_out[4], is_out2[4];
	void (*fclip_point)(struct view *view, struct m1_point *pt, struct m1_point *p1, struct m1_point *p2);

	if(level == 4) {
		LOG_TGP(("VIDEOCQ %d", level));
		for(i=0; i<4; i++)
			LOG_TGP((" (%f, %f, %f)", q->p[i]->x, q->p[i]->y, q->p[i]->z));
		LOG_TGP(("\n"));
		*m_quadpt = *q;
		m_quadpt++;
		return;
	}

	for(i=0; i<4; i++)
		is_out[i] = clipfn[level].isclipped(view, q->p[i]);

	LOG_TGP(("VIDEOCQ %d", level));
	for(i=0; i<4; i++)
		LOG_TGP((" (%f, %f, %f, %d)", q->p[i]->x, q->p[i]->y, q->p[i]->z, is_out[i]));
	LOG_TGP(("\n"));

	// No clipping
	if(!is_out[0] && !is_out[1] && !is_out[2] && !is_out[3]) {
		fclip_push_quad(level+1, q);
		return;
	}

	fclip_point = clipfn[level].clip;

	// Full clipping
	if(is_out[0] && is_out[1] && is_out[2] && is_out[3])
		return;

	// Find n so that point n is clipped and n-1 isn't
	for(i=0; i<4; i++)
		if(is_out[i] && !is_out[(i-1)&3])
			break;

	for(j=0; j<4; j++) {
		pt[j] = q->p[(i+j)&3];
		is_out2[j] = is_out[(i+j)&3];
	}

	// At this point, pt[0] is clipped out and pt[3] isn't.  Test for the 4 possible cases
	if(is_out2[1])
		if(is_out2[2]) {
			// pt 0,1,2 clipped out, one triangle left
			fclip_point(view, m_pointpt, pt[2], pt[3]);
			pi1 = m_pointpt++;
			fclip_point(view, m_pointpt, pt[3], pt[0]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[3], pi2, pi2);
		} else {
			// pt 0,1 clipped out, one quad left
			fclip_point(view, m_pointpt, pt[1], pt[2]);
			pi1 = m_pointpt++;
			fclip_point(view, m_pointpt, pt[3], pt[0]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[2], pt[3], pi2);
		}
	else
		if(is_out2[2]) {
			// pt 0,2 clipped out, shouldn't happen, two triangles
			fclip_point(view, m_pointpt, pt[0], pt[1]);
			pi1 = m_pointpt++;
			fclip_point(view, m_pointpt, pt[1], pt[2]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[1], pi2, pi2);
			fclip_point(view, m_pointpt, pt[2], pt[3]);
			pi1 = m_pointpt++;
			fclip_point(view, m_pointpt, pt[3], pt[0]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[3], pi2, pi2);
		} else {
			// pt 0 clipped out, one decagon left, split in quad+tri
			fclip_point(view, m_pointpt, pt[0], pt[1]);
			pi1 = m_pointpt++;
			fclip_point(view, m_pointpt, pt[3], pt[0]);
			pi2 = m_pointpt++;
			fclip_push_quad_next(level, q, pi1, pt[1], pt[2], pt[3]);
			fclip_push_quad_next(level, q, pt[3], pi2, pi1, pi1);
		}
}

static float min4f(float a, float b, float c, float d)
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

static float max4f(float a, float b, float c, float d)
{
	float m = a;
	if(b>m)
		m = b;
	if(c>m)
		m = c;
	if(d>m)
		m = d;
	return m;
}

#ifdef UNUSED_DEFINITION
static const UINT8 num_of_times[]={1,1,1,1,2,2,2,3};
#endif
static float compute_specular(struct vector_t *normal, struct vector_t *light,float diffuse,int lmode)
{
#if 0
	float s;
	int p=view->lightparams[lmode].p&7;
	int i;
	float sv=view->lightparams[lmode].s;

	//This is how it should be according to model2 geo program, but doesn't work fine
	s=2*(diffuse*normal->z-light->z);
	for(i=0;i<num_of_times[p];++i)
		s*=s;
	s*=sv;
	if(s<0.0)
		return 0.0;
	if(s>1.0)
		return 1.0;
	return s;

	return fabs(diffuse)*sv;

#endif

	return 0;
}

void model1_state::push_object(UINT32 tex_adr, UINT32 poly_adr, UINT32 size)
{
	struct view *view = m_view;
	int i;
	UINT32 flags;
	struct m1_point *old_p0, *old_p1, *p0, *p1;
	struct vector_t vn;
	int link, type;
#if 0
	int dump;
#endif
	int lightmode;
	float old_z;
	struct quad_m1 cquad;
	float *poly_data;

	if(poly_adr & 0x800000)
		poly_data=(float *) m_poly_ram;
	else
		poly_data=(float *) m_poly_rom;

	poly_adr &= 0x7fffff;
#if 0
	dump = poly_adr == 0x944ea;
	dump = 0;
#endif

#if 0
	if(poly_adr < 0x10000 || poly_adr >= 0x80000)
		return;

	if(poly_adr < 0x40000 || poly_adr >= 0x50000)
		return;

	if(poly_adr == 0x4c7db || poly_adr == 0x4cdaa || poly_adr == 0x4d3e7)
		return;

	if(poly_adr != 0x483e5)
		return;
#endif

	if(1) {
		LOG_TGP(("XVIDEO:   draw object (%x, %x, %x)\n", tex_adr, poly_adr, size));
	}

	if(!size)
		size = 0xffffffff;

	old_p0 = m_pointpt++;
	old_p1 = m_pointpt++;

	old_p0->x = poly_data[poly_adr+0];
	old_p0->y = poly_data[poly_adr+1];
	old_p0->z = poly_data[poly_adr+2];
	old_p1->x = poly_data[poly_adr+3];
	old_p1->y = poly_data[poly_adr+4];
	old_p1->z = poly_data[poly_adr+5];
	_transform_point(view, old_p0);
	_transform_point(view, old_p1);
	if(old_p0->z > 0)
		project_point(view, old_p0);
	else
		old_p0->s.x = old_p0->s.y = 0;
	if(old_p1->z > 0)
		project_point(view, old_p1);
	else
		old_p1->s.x = old_p1->s.y = 0;

	old_z = 0;

	poly_adr += 6;

	for(i=0; i<size; i++) {
#if 0
		LOG_TGP(("VIDEO:     %08x (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)\n",
					*(UINT32 *)(poly_data+poly_adr) & ~(0x01800303),
					poly_data[poly_adr+1], poly_data[poly_adr+2], poly_data[poly_adr+3],
					poly_data[poly_adr+4], poly_data[poly_adr+5], poly_data[poly_adr+6],
					poly_data[poly_adr+7], poly_data[poly_adr+8], poly_data[poly_adr+9]));
#endif
		flags = *(UINT32 *)(poly_data+poly_adr);

		type = flags & 3;
		if(!type)
			break;

		if(flags & 0x00001000)
			tex_adr ++;
		lightmode=(flags>>17)&15;

		p0 = m_pointpt++;
		p1 = m_pointpt++;

		vn.x = poly_data[poly_adr+1];
		vn.y = poly_data[poly_adr+2];
		vn.z = poly_data[poly_adr+3];
		p0->x = poly_data[poly_adr+4];
		p0->y = poly_data[poly_adr+5];
		p0->z = poly_data[poly_adr+6];
		p1->x = poly_data[poly_adr+7];
		p1->y = poly_data[poly_adr+8];
		p1->z = poly_data[poly_adr+9];

		link = (flags >> 8) & 3;

		transform_vector(view, &vn);

		_transform_point(view, p0);
		_transform_point(view, p1);
		if(p0->z > 0)
			project_point(view, p0);
		else
			p0->s.x = p0->s.y = 0;
		if(p1->z > 0)
			project_point(view, p1);
		else
			p1->s.x = p1->s.y = 0;

#if 0
		if(dump)
			LOG_TGP(("VIDEO:     %08x (%f, %f, %f) (%f, %f, %f)\n",
						*(UINT32 *)(poly_data+poly_adr),
						p0->x, p0->y, p0->z,
						p1->x, p1->y, p1->z));
#endif


#if 0
		if(1 || dump) {
			LOG_TGP(("VIDEO:     %08x (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n",
						*(UINT32 *)(poly_data+poly_adr),
						old_p0->s.x, old_p0->s.y,
						old_p1->s.x, old_p1->s.y,
						p0->s.x, p0->s.y,
						p1->s.x, p1->s.y));
		}
#endif

		if(!link)
			goto next;

		if(!(flags & 0x00004000) && view_determinant(old_p1, old_p0, p0) > 0)
			goto next;

		normalize_vector(&vn);

		cquad.p[0] = old_p1;
		cquad.p[1] = old_p0;
		cquad.p[2] = p0;
		cquad.p[3] = p1;

		switch((flags >> 10) & 3) {
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
			float dif=mult_vector(&vn, &view->light);
			float ln=view->lightparams[lightmode].a + view->lightparams[lightmode].d*MAX(0.0,dif);
			cquad.col = scale_color(machine().pens[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)], MIN(1.0,ln));
			cquad.col = scale_color(machine().pens[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)], MIN(1.0,ln));
#endif
			float dif=mult_vector(&vn, &view->light);
			float spec=compute_specular(&vn,&view->light,dif,lightmode);
			float ln=view->lightparams[lightmode].a + view->lightparams[lightmode].d*MAX(0.0f,dif) + spec;
			int lumval=255.0f*MIN(1.0f,ln);
			int color=m_paletteram16[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)];
			int r=(color>>0x0)&0x1f;
			int g=(color>>0x5)&0x1f;
			int b=(color>>0xA)&0x1f;
			lumval>>=2; //there must be a luma translation table somewhere
			if(lumval>0x3f) lumval=0x3f;
			else if(lumval<0) lumval=0;
			r=(m_color_xlat[(r<<8)|lumval|0x0]>>3)&0x1f;
			g=(m_color_xlat[(g<<8)|lumval|0x2000]>>3)&0x1f;
			b=(m_color_xlat[(b<<8)|lumval|0x4000]>>3)&0x1f;
			cquad.col=(pal5bit(r)<<16)|(pal5bit(g)<<8)|(pal5bit(b)<<0);
		}

		if(flags & 0x00002000)
			cquad.col |= MOIRE;

		fclip_push_quad(0, &cquad);

	next:
		poly_adr += 10;
		switch(link) {
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

UINT16 *model1_state::push_direct(UINT16 *list)
{
	struct view *view = m_view;
	UINT32 flags;
	UINT32 tex_adr, lum; //, v1, v2;
	struct m1_point *old_p0, *old_p1, *p0, *p1;
	int link, type;
	float z;
	struct quad_m1 cquad;

	tex_adr = readi(list);
//  v1      = readi(list+2);
//  v2      = readi(list+10);

	old_p0 = m_pointpt++;
	old_p1 = m_pointpt++;

	old_p0->x = readf(list+4);
	old_p0->y = readf(list+6);
	old_p0->z = readf(list+8);
	old_p1->x = readf(list+12);
	old_p1->y = readf(list+14);
	old_p1->z = readf(list+16);

	LOG_TGP(("VIDEOD start direct\n"));
	LOG_TGP(("VIDEOD (%f, %f, %f) (%f, %f, %f)\n",
				old_p0->x, old_p0->y, old_p0->z,
				old_p1->x, old_p1->y, old_p1->z));

	//_transform_point(view, old_p0);
	//_transform_point(view, old_p1);
	if(old_p0->z > 0)
		project_point_direct(view, old_p0);
	else
		old_p0->s.x = old_p0->s.y = 0;
	if(old_p1->z > 0)
		project_point_direct(view, old_p1);
	else
		old_p1->s.x = old_p1->s.y = 0;

	list += 18;

	for(;;) {
		flags = readi(list);

		type = flags & 3;
		if(!type)
			break;

		if(flags & 0x00001000)
			tex_adr ++;

		// list+2 is luminosity
		// list+4 is 0?
		// list+12 is z?

		p0 = m_pointpt++;
		p1 = m_pointpt++;

		lum   = readi(list+2);
//      v1    = readi(list+4);

		if(type == 2) {
			p0->x = readf(list+6);
			p0->y = readf(list+8);
			p0->z = readf(list+10);
			z = p0->z;
			LOG_TGP(("VIDEOD %08x %08x (%f, %f, %f)\n",
						flags, lum,
						p0->x, p0->y, p0->z));
			*p1 = *p0;
			list += 12;
		} else {
			p0->x = readf(list+6);
			p0->y = readf(list+8);
			p0->z = readf(list+10);
			p1->x = readf(list+14);
			p1->y = readf(list+16);
			p1->z = readf(list+18);
			z     = readf(list+12);
			LOG_TGP(("VIDEOD %08x %08x (%f, %f, %f) (%f, %f, %f)\n",
						flags, lum,
						p0->x, p0->y, p0->z,
						p1->x, p1->y, p1->z));
			list += 20;
		}

		link = (flags >> 8) & 3;

		//_transform_point(view, p0);
		//_transform_point(view, p1);
		if(p0->z > 0)
			project_point_direct(view, p0);
		if(p1->z > 0)
			project_point_direct(view, p1);

#if 1
		if(old_p0 && old_p1)
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

		if(!link)
			goto next;

		cquad.p[0] = old_p1;
		cquad.p[1] = old_p0;
		cquad.p[2] = p0;
		cquad.p[3] = p1;
		cquad.z    = z;
		{
			int lumval=((float) (lum>>24)) * 2.0f;
			int color=m_paletteram16[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)];
			int r=(color>>0x0)&0x1f;
			int g=(color>>0x5)&0x1f;
			int b=(color>>0xA)&0x1f;
			lumval>>=2; //there must be a luma translation table somewhere
			if(lumval>0x3f) lumval=0x3f;
			else if(lumval<0) lumval=0;
			r=(m_color_xlat[(r<<8)|lumval|0x0]>>3)&0x1f;
			g=(m_color_xlat[(g<<8)|lumval|0x2000]>>3)&0x1f;
			b=(m_color_xlat[(b<<8)|lumval|0x4000]>>3)&0x1f;
			cquad.col=(pal5bit(r)<<16)|(pal5bit(g)<<8)|(pal5bit(b)<<0);
		}
		//cquad.col  = scale_color(machine().pens[0x1000|(m_tgp_ram[tex_adr-0x40000] & 0x3ff)],((float) (lum>>24)) / 128.0);
		if(flags & 0x00002000)
			cquad.col |= MOIRE;

		fclip_push_quad(0, &cquad);

	next:
		switch(link) {
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
	return list+2;
}

static UINT16 *skip_direct(UINT16 *list)
{
	UINT32 flags;
	int type;

	list += 18;

	for(;;) {
		flags = readi(list);

		type = flags & 3;
		if(!type)
			break;

		if(type == 2)
			list += 12;
		else
			list += 20;
	}
	return list+2;
}

void model1_state::draw_objects(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(m_quadpt != m_quaddb) {
		LOG_TGP(("VIDEO: sort&draw\n"));
		sort_quads();
		draw_quads(bitmap, cliprect);
	}

	m_quadpt = m_quaddb;
	m_pointpt = m_pointdb;
}

UINT16 *model1_state::draw_direct(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 *list)
{
	UINT16 *res;

	LOG_TGP(("VIDEO:   draw direct %x\n", readi(list)));

	draw_objects(bitmap, cliprect);
	res = push_direct(list);
	unsort_quads();
	draw_quads(bitmap, cliprect);

	m_quadpt = m_quaddb;
	m_pointpt = m_pointdb;
	return res;
}

UINT16 *model1_state::get_list()
{
	if(!(m_listctl[0] & 4))
		m_listctl[0] = (m_listctl[0] & ~0x40) | (m_listctl[0] & 8 ? 0x40 : 0);
	return m_listctl[0] & 0x40 ? m_display_list1 : m_display_list0;
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
	COMBINE_DATA(m_listctl+offset);
	LOG_TGP(("VIDEO: control=%08x\n", (m_listctl[1]<<16)|m_listctl[0]));
}

void model1_state::tgp_render(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	struct view *view = m_view;
	m_render_done = 1;
	if((m_listctl[1] & 0x1f) == 0x1f) {
		UINT16 *list = get_list();
		int zz = 0;
		LOG_TGP(("VIDEO: render list %d\n", get_list_number()));

		memset(view->trans_mat, 0, sizeof(view->trans_mat));
		view->trans_mat[0] = 1.0;
		view->trans_mat[4] = 1.0;
		view->trans_mat[8] = 1.0;

		for(;;) {
			int type = (list[1]<<16)|list[0];
			m_glist=list;
			switch(type & 15) {
			case 0:
				list += 2;
				break;
			case 1:
				// 1 = plane 1
				// 2 = ??  draw object (413d3, 17c4c, e)
				// 3 = plane 2
				// 4 = ??  draw object (408a8, a479, 9)
				// 5 = decor
				// 6 = ??  draw object (57bd4, 387460, 2ad)

				if(1 || zz >= 666)
					push_object(readi(list+2), readi(list+4), readi(list+6));
				list += 8;
				break;
			case 2:
				list = draw_direct(bitmap, cliprect, list+2);
				break;
			case 3:
				LOG_TGP(("VIDEO:   viewport (%d, %d, %d, %d, %d, %d, %d)\n",
							readi(list+2),
							readi16(list+4), readi16(list+6), readi16(list+8),
							readi16(list+10), readi16(list+12), readi16(list+14)));

				draw_objects(bitmap, cliprect);

				view->xc = readi16(list+4);
				view->yc = 383-(readi16(list+6)-39);
				view->x1 = readi16(list+8);
				view->y2 = 383-(readi16(list+10)-39);
				view->x2 = readi16(list+12);
				view->y1 = 383-(readi16(list+14)-39);

				recompute_frustrum(view);

				list += 16;
				break;
			case 4: {
				int adr = readi(list+2);
				int len = readi(list+4)+1;
				int i;
				LOG_TGP(("ZVIDEO:   color write, adr=%x, len=%x\n", adr, len));
				for(i=0; i<len; i++)
					m_tgp_ram[adr-0x40000+i] = list[6+2*i];
				list += 6+len*2;
				break;
			}
			case 5:
				{
					int adr = readi(list+2);
					int len = readi(list+4);
					int i;
					for(i=0;i<len;++i)
					{
						m_poly_ram[adr-0x800000+i]=readi(list+2*i+6);
					}
					list+=6+len*2;
				}
				break;
			case 6: {
				int adr = readi(list+2);
				int len = readi(list+4);
				int i;
				LOG_TGP(("VIDEO:   upload data, adr=%x, len=%x\n", adr, len));
				for(i=0;i<len;++i)
				{
					int v=readi(list+6+i*2);
					view->lightparams[i+adr].d=((float) (v&0xff))/255.0f;
					view->lightparams[i+adr].a=((float) ((v>>8)&0xff))/255.0f;
					view->lightparams[i+adr].s=((float) ((v>>16)&0xff))/255.0f;
					view->lightparams[i+adr].p=(v>>24)&0xff;
				}
				list += 6+len*2;
				break;
			}
			case 7:
				LOG_TGP(("VIDEO:   code 7 (%d)\n", readi(list+2)));
				zz++;
				list += 4;
				break;
			case 8:
				LOG_TGP(("VIDEO:   select mode %08x\n", readi(list+2)));
				list += 4;
				break;
			case 9:
				LOG_TGP(("VIDEO:   zoom (%f, %f)\n", readf(list+2), readf(list+4)));
				view->zoomx = readf(list+2)*4;
				view->zoomy = readf(list+4)*4;

				recompute_frustrum(view);

				list += 6;
				break;
			case 0xa:
				LOG_TGP(("VIDEO:   light vector (%f, %f, %f)\n", readf(list+2), readf(list+4), readf(list+6)));
				view->light.x = readf(list+2);
				view->light.y = readf(list+4);
				view->light.z = readf(list+6);
				normalize_vector(&view->light);
				list += 8;
				break;
			case 0xb: {
				int i;
				LOG_TGP(("VIDEO:   matrix (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
							readf(list+2), readf(list+4), readf(list+6),
							readf(list+8), readf(list+10), readf(list+12),
							readf(list+14), readf(list+16), readf(list+18),
							readf(list+20), readf(list+22), readf(list+24)));
				for(i=0; i<12; i++)
					view->trans_mat[i] = readf(list+2+2*i);
				list += 26;
				break;
			}
			case 0xc:
				LOG_TGP(("VIDEO:   trans (%f, %f)\n", readf(list+2), readf(list+4)));
				view->transx = readf(list+2);
				view->transy = readf(list+4);

				recompute_frustrum(view);

				list += 6;
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
	struct view *view = m_view;
#if 0
	if (machine().input().code_pressed_once(KEYCODE_F))
		{
		FILE *fp;
		fp=fopen("tgp-ram.bin", "w+b");
		if (fp)
				{
			fwrite(m_tgp_ram, (0x100000-0x40000)*2, 1, fp);
			fclose(fp);
				}
		exit(0);
		}
#endif
	if(!m_render_done && (m_listctl[1] & 0x1f) == 0x1f) {
		UINT16 *list = get_list();
		// Skip everything but the data uploads
		LOG_TGP(("VIDEO: scan list %d\n", get_list_number()));
		for(;;) {
			int type = (list[1]<<16)|list[0];
			switch(type) {
			case 0:
				list += 2;
				break;
			case 1:
				list += 8;
				break;
			case 2:
				list = skip_direct(list+2);
				break;
			case 3:
				list += 16;
				break;
			case 4: {
				int adr = readi(list+2);
				int len = readi(list+4)+1;
				int i;
				LOG_TGP(("ZVIDEO:   scan color write, adr=%x, len=%x\n", adr, len));
				for(i=0; i<len; i++)
					m_tgp_ram[adr-0x40000+i] = list[6+2*i];
				list += 6+len*2;
				break;
			}
			case 5:
				{
					int adr = readi(list+2);
					int len = readi(list+4);
					int i;
					for(i=0;i<len;++i)
					{
						m_poly_ram[adr-0x800000+i]=readi(list+2*i+6);
					}
					list+=6+len*2;
				}
				break;
			case 6: {
				int adr = readi(list+2);
				int len = readi(list+4);
				int i;
				//LOG_TGP(("VIDEO:   upload data, adr=%x, len=%x\n", adr, len));
				for(i=0;i<len;++i)
				{
					int v=readi(list+6+i*2);
					view->lightparams[i+adr].d=((float) (v&0xff))/255.0f;
					view->lightparams[i+adr].a=((float) ((v>>8)&0xff))/255.0f;
					view->lightparams[i+adr].s=((float) ((v>>16)&0xff))/255.0f;
					view->lightparams[i+adr].p=(v>>24)&0xff;
					//LOG_TGP(("         %02X\n",v));
				}
				list += 6+len*2;
				break;
			}
			case 7:
				list += 4;
				break;
			case 8:
				list += 4;
				break;
			case 9:
				list += 6;
				break;
			case 0xa:
				list += 8;
				break;
			case 0xb:
				list += 26;
				break;
			case 0xc:
				list += 6;
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

VIDEO_START_MEMBER(model1_state,model1)
{
	m_view = auto_alloc_clear(machine(), struct view);

	m_poly_rom = (UINT32 *)memregion("user1")->base();
	m_poly_ram = auto_alloc_array_clear(machine(), UINT32, 0x400000);
	m_tgp_ram = auto_alloc_array_clear(machine(), UINT16, 0x100000-0x40000);
	m_pointdb = auto_alloc_array_clear(machine(), struct m1_point, 1000000*2);
	m_quaddb  = auto_alloc_array_clear(machine(), struct quad_m1, 1000000);
	m_quadind = auto_alloc_array_clear(machine(), struct quad_m1 *, 1000000);

	m_pointpt = m_pointdb;
	m_quadpt = m_quaddb;
	m_listctl[0] = m_listctl[1] = 0;

	save_pointer(NAME(m_tgp_ram), 0x100000-0x40000);
	save_pointer(NAME(m_poly_ram), 0x40000);
	save_item(NAME(m_listctl));
}

UINT32 model1_state::screen_update_model1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	struct view *view = m_view;
#if 0
	{
		int mod = 0;
		double delta;
		delta = 1;

		if(machine().input().code_pressed(KEYCODE_F)) {
			mod = 1;
			view->vxx -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_G)) {
			mod = 1;
			view->vxx += delta;
		}
		if(machine().input().code_pressed(KEYCODE_H)) {
			mod = 1;
			view->vyy -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_J)) {
			mod = 1;
			view->vyy += delta;
		}
		if(machine().input().code_pressed(KEYCODE_K)) {
			mod = 1;
			view->vzz -= delta;
		}
		if(machine().input().code_pressed(KEYCODE_L)) {
			mod = 1;
			view->vzz += delta;
		}
		if(machine().input().code_pressed(KEYCODE_U)) {
			mod = 1;
			view->ayy -= 0.05;
		}
		if(machine().input().code_pressed(KEYCODE_I)) {
			mod = 1;
			view->ayy += 0.05;
		}
		if(mod)
			popmessage("%g,%g,%g:%g", view->vxx, view->vyy, view->vzz, view->ayy);
	}
#endif

	view->ayyc = cos(view->ayy);
	view->ayys = sin(view->ayy);

	screen.priority().fill(0);
	bitmap.fill(m_palette->pen(0), cliprect);

	segas24_tile *tile = machine().device<segas24_tile>("tile");
	tile->draw(screen, bitmap, cliprect, 6, 0, 0);
	tile->draw(screen, bitmap, cliprect, 4, 0, 0);
	tile->draw(screen, bitmap, cliprect, 2, 0, 0);
	tile->draw(screen, bitmap, cliprect, 0, 0, 0);

	tgp_render(bitmap, cliprect);

	tile->draw(screen, bitmap, cliprect, 7, 0, 0);
	tile->draw(screen, bitmap, cliprect, 5, 0, 0);
	tile->draw(screen, bitmap, cliprect, 3, 0, 0);
	tile->draw(screen, bitmap, cliprect, 1, 0, 0);

	return 0;
}

void model1_state::screen_eof_model1(screen_device &screen, bool state)
{
	// on rising edge
	if (state)
	{
		tgp_scan();
		end_frame();
		LOG_TGP(("TGP: vsync\n"));
	}
}
