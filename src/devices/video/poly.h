// license:BSD-3-Clause
// copyright-holders:Ville Linde, Aaron Giles
/***************************************************************************

    poly.h

    Polygon helper routines.

****************************************************************************

    Pixel model:

    (0.0,0.0)       (1.0,0.0)       (2.0,0.0)       (3.0,0.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,0.5)   |   (1.5,0.5)   |   (2.5,0.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
    (0.0,1.0)       (1.0,1.0)       (2.0,1.0)       (3.0,1.0)
        +---------------+---------------+---------------+
        |               |               |               |
        |               |               |               |
        |   (0.5,1.5)   |   (1.5,1.5)   |   (2.5,1.5)   |
        |       *       |       *       |       *       |
        |               |               |               |
        |               |               |               |
        |               |               |               |
        +---------------+---------------+---------------+
    (0.0,2.0)       (1.0,2.0)       (2.0,2.0)       (3.0,2.0)

***************************************************************************/

#pragma once

#ifndef __POLY_H__
#define __POLY_H__

#include <limits.h>

//**************************************************************************
//  DEBUGGING
//**************************************************************************

// keep statistics
#define KEEP_POLY_STATISTICS            0

// turn this on to log the reasons for any long waits
#define LOG_WAITS                       0

// number of profiling ticks before we consider a wait "long"
#define LOG_WAIT_THRESHOLD              1000



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define POLYFLAG_INCLUDE_BOTTOM_EDGE        0x01
#define POLYFLAG_INCLUDE_RIGHT_EDGE         0x02
#define POLYFLAG_NO_WORK_QUEUE              0x04

#define SCANLINES_PER_BUCKET                8
#define CACHE_LINE_SIZE                     64          // this is a general guess
#define TOTAL_BUCKETS                       (512 / SCANLINES_PER_BUCKET)
#define UNITS_PER_POLY                      (100 / SCANLINES_PER_BUCKET)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  global helpers for float base types
//-------------------------------------------------

inline float poly_floor(float x) { return floorf(x); }
inline float poly_abs(float x) { return fabsf(x); }
inline float poly_recip(float x) { return 1.0f / x; }


//-------------------------------------------------
//  global helpers for double base types
//-------------------------------------------------

inline double poly_floor(double x) { return floor(x); }
inline double poly_abs(double x) { return fabs(x); }
inline double poly_recip(double x) { return 1.0 / x; }


// poly_manager is a template class
template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
class poly_manager
{
public:
	// each vertex has an X/Y coordinate and a set of parameters
	struct vertex_t
	{
		vertex_t() { }
		vertex_t(_BaseType _x, _BaseType _y) { x = _x; y = _y; }

		_BaseType x, y;                         // X, Y coordinates
		_BaseType p[_MaxParams];                // interpolated parameters
	};

	// a single extent describes a span and a list of parameter extents
	struct extent_t
	{
		INT16 startx, stopx;                    // starting (inclusive)/ending (exclusive) endpoints
		struct
		{
			_BaseType start;                    // parameter value at start
			_BaseType dpdx;                     // dp/dx relative to start
		} param[_MaxParams];
		void *userdata;                         // custom per-span data
	};

	// delegate type for scanline callbacks
	typedef delegate<void (INT32, const extent_t &, const _ObjectData &, int)> render_delegate;

	// construction/destruction
	poly_manager(running_machine &machine, UINT8 flags = 0);
	poly_manager(screen_device &screen, UINT8 flags = 0);
	virtual ~poly_manager();

	// getters
	running_machine &machine() const { return m_machine; }
	screen_device &screen() const { assert(m_screen != nullptr); return *m_screen; }

	// synchronization
	void wait(const char *debug_reason = "general");

	// object data allocators
	_ObjectData &object_data_alloc();
	_ObjectData &object_data_last() const { return m_object.last(); }

	// tiles
	UINT32 render_tile(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t &v1, const vertex_t &v2);

	// triangles
	UINT32 render_triangle(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t &v1, const vertex_t &v2, const vertex_t &v3);
	UINT32 render_triangle_fan(const rectangle &cliprect, render_delegate callback, int paramcount, int numverts, const vertex_t *v);
	UINT32 render_triangle_strip(const rectangle &cliprect, render_delegate callback, int paramcount, int numverts, const vertex_t *v);
	UINT32 render_triangle_custom(const rectangle &cliprect, render_delegate callback, int startscanline, int numscanlines, const extent_t *extents);

	// polygons
	template<int _NumVerts>
	UINT32 render_polygon(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t *v);

	// public helpers
	int zclip_if_less(int numverts, const vertex_t *v, vertex_t *outv, int paramcount, _BaseType clipval);

private:
	// polygon_info describes a single polygon, which includes the poly_params
	struct polygon_info
	{
		poly_manager *      m_owner;                // pointer back to the poly manager
		_ObjectData *       m_object;               // object data pointer
		render_delegate     m_callback;             // callback to handle a scanline's worth of work
	};

	// internal unit of work
	struct work_unit
	{
		volatile UINT32     count_next;             // number of scanlines and index of next item to process
		polygon_info *      polygon;                // pointer to polygon
		INT16               scanline;               // starting scanline
		UINT16              previtem;               // index of previous item in the same bucket
	#ifndef PTR64
		UINT32              dummy;                  // pad to 16 bytes
	#endif
		extent_t            extent[SCANLINES_PER_BUCKET]; // array of scanline extents
	};

	// class for managing an array of items
	template<class _Type, int _Count>
	class poly_array
	{
		// size of an item, rounded up to the cache line size
		static const int k_itemsize = ((sizeof(_Type) + CACHE_LINE_SIZE - 1) / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;

	public:
		// construction
		poly_array(running_machine &machine, poly_manager &manager)
			: m_manager(manager),
				m_base(auto_alloc_array_clear(machine, UINT8, k_itemsize * _Count)),
				m_next(0),
				m_max(0),
				m_waits(0) { }

		// destruction
		~poly_array() { auto_free(m_manager.machine(), m_base); }

		// operators
		_Type &operator[](int index) const { assert(index >= 0 && index < _Count); return *reinterpret_cast<_Type *>(m_base + index * k_itemsize); }

		// getters
		int count() const { return m_next; }
		int max() const { return m_max; }
		int waits() const { return m_waits; }
		int itemsize() const { return k_itemsize; }
		int allocated() const { return _Count; }
		int indexof(_Type &item) const { int result = (reinterpret_cast<UINT8 *>(&item) - m_base) / k_itemsize; assert(result >= 0 && result < _Count); return result; }

		// operations
		void reset() { m_next = 0; }
		_Type &next() { if (m_next > m_max) m_max = m_next; assert(m_next < _Count); return *new(m_base + m_next++ * k_itemsize) _Type; }
		_Type &last() const { return (*this)[m_next - 1]; }
		void wait_for_space(int count = 1) { while ((m_next + count) >= _Count) { m_waits++; m_manager.wait(""); }  }

	private:
		// internal state
		poly_manager &      m_manager;
		UINT8 *             m_base;
		int                 m_next;
		int                 m_max;
		int                 m_waits;
	};

	// internal array types
	typedef poly_array<polygon_info, _MaxPolys> polygon_array;
	typedef poly_array<_ObjectData, _MaxPolys + 1> objectdata_array;
	typedef poly_array<work_unit, MIN(_MaxPolys * UNITS_PER_POLY, 65535)> unit_array;

	// round in a cross-platform consistent manner
	inline INT32 round_coordinate(_BaseType value)
	{
		INT32 result = poly_floor(value);

		if ((value > 0) && (result < 0))
			return INT_MAX-1;
		return result + (value - _BaseType(result) > _BaseType(0.5));
	}

	// internal helpers
	polygon_info &polygon_alloc(int minx, int maxx, int miny, int maxy, render_delegate callback)
	{
		// wait for space in the polygon and unit arrays
		m_polygon.wait_for_space();
		m_unit.wait_for_space((maxy - miny) / SCANLINES_PER_BUCKET + 2);

		// return and initialize the next one
		polygon_info &polygon = m_polygon.next();
		polygon.m_owner = this;
		polygon.m_object = &object_data_last();
		polygon.m_callback = callback;
		return polygon;
	}

	static void *work_item_callback(void *param, int threadid);
	void presave() { wait("pre-save"); }

	// queue management
	running_machine &   m_machine;
	screen_device *     m_screen;
	osd_work_queue *    m_queue;                    // work queue

	// arrays
	polygon_array       m_polygon;                  // array of polygons
	objectdata_array    m_object;                   // array of object data
	unit_array          m_unit;                     // array of work units

	// misc data
	UINT8               m_flags;                    // flags

	// buckets
	UINT16              m_unit_bucket[TOTAL_BUCKETS]; // buckets for tracking unit usage

	// statistics
	UINT32              m_tiles;                    // number of tiles queued
	UINT32              m_triangles;                // number of triangles queued
	UINT32              m_quads;                    // number of quads queued
	UINT64              m_pixels;                   // number of pixels rendered
#if KEEP_POLY_STATISTICS
	UINT32              m_conflicts[WORK_MAX_THREADS]; // number of conflicts found, per thread
	UINT32              m_resolved[WORK_MAX_THREADS];   // number of conflicts resolved, per thread
#endif
};


//-------------------------------------------------
//  poly_manager - constructor
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::poly_manager(running_machine &machine, UINT8 flags)
	: m_machine(machine),
		m_screen(nullptr),
		m_queue(nullptr),
		m_polygon(machine, *this),
		m_object(machine, *this),
		m_unit(machine, *this),
		m_flags(flags),
		m_triangles(0),
		m_quads(0),
		m_pixels(0)
{
#if KEEP_POLY_STATISTICS
	memset(m_conflicts, 0, sizeof(m_conflicts));
	memset(m_resolved, 0, sizeof(m_resolved));
#endif

	// create the work queue
	if (!(flags & POLYFLAG_NO_WORK_QUEUE))
		m_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	// request a pre-save callback for synchronization
	machine.save().register_presave(save_prepost_delegate(FUNC(poly_manager::presave), this));
}


template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::poly_manager(screen_device &screen, UINT8 flags)
	: m_machine(screen.machine()),
		m_screen(&screen),
		m_queue(nullptr),
		m_polygon(screen.machine(), *this),
		m_object(screen.machine(), *this),
		m_unit(screen.machine(), *this),
		m_flags(flags),
		m_triangles(0),
		m_quads(0),
		m_pixels(0)
{
#if KEEP_POLY_STATISTICS
	memset(m_conflicts, 0, sizeof(m_conflicts));
	memset(m_resolved, 0, sizeof(m_resolved));
#endif

	// create the work queue
	if (!(flags & POLYFLAG_NO_WORK_QUEUE))
		m_queue = osd_work_queue_alloc(WORK_QUEUE_FLAG_MULTI | WORK_QUEUE_FLAG_HIGH_FREQ);

	// request a pre-save callback for synchronization
	machine().save().register_presave(save_prepost_delegate(FUNC(poly_manager::presave), this));
}


//-------------------------------------------------
//  ~poly_manager - destructor
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::~poly_manager()
{
#if KEEP_POLY_STATISTICS
{
	// accumulate stats over the entire collection
	int conflicts = 0, resolved = 0;
	for (int i = 0; i < ARRAY_LENGTH(m_conflicts); i++)
	{
		conflicts += m_conflicts[i];
		resolved += m_resolved[i];
	}

	// output global stats
	printf("Total triangles = %d\n", m_triangles);
	printf("Total quads = %d\n", m_quads);
	if (m_pixels > 1000000000)
		printf("Total pixels   = %d%09d\n", (UINT32)(m_pixels / 1000000000), (UINT32)(m_pixels % 1000000000));
	else
		printf("Total pixels   = %d\n", (UINT32)m_pixels);

	printf("Conflicts:   %d resolved, %d total\n", resolved, conflicts);
	printf("Units:       %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", m_unit.max(), m_unit.allocated(), m_unit.waits(), m_unit.itemsize(), m_unit.allocated() * m_unit.itemsize());
	printf("Polygons:    %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", m_polygon.max(), m_polygon.allocated(), m_polygon.waits(), m_polygon.itemsize(), m_polygon.allocated() * m_polygon.itemsize());
	printf("Object data: %5d used, %5d allocated, %5d waits, %4d bytes each, %7d total\n", m_object.max(), m_object.allocated(), m_object.waits(), m_object.itemsize(), m_object.allocated() * m_object.itemsize());
}
#endif

	// free the work queue
	if (m_queue != nullptr)
		osd_work_queue_free(m_queue);
}


//-------------------------------------------------
//  work_item_callback - process a work item
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
void *poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::work_item_callback(void *param, int threadid)
{
	while (1)
	{
		work_unit &unit = *(work_unit *)param;
		polygon_info &polygon = *unit.polygon;
		int count = unit.count_next & 0xffff;
		UINT32 orig_count_next;

		// if our previous item isn't done yet, enqueue this item to the end and proceed
		if (unit.previtem != 0xffff)
		{
			work_unit &prevunit = polygon.m_owner->m_unit[unit.previtem];
			if (prevunit.count_next != 0)
			{
				UINT32 unitnum = polygon.m_owner->m_unit.indexof(unit);
				UINT32 new_count_next;

				// attempt to atomically swap in this new value
				do
				{
					orig_count_next = prevunit.count_next;
					new_count_next = orig_count_next | (unitnum << 16);
				} while (compare_exchange32((volatile INT32 *)&prevunit.count_next, orig_count_next, new_count_next) != orig_count_next);

#if KEEP_POLY_STATISTICS
				// track resolved conflicts
				polygon.m_owner->m_conflicts[threadid]++;
				if (orig_count_next != 0)
					polygon.m_owner->m_resolved[threadid]++;
#endif
				// if we succeeded, skip out early so we can do other work
				if (orig_count_next != 0)
					break;
			}
		}

		// iterate over extents
		for (int curscan = 0; curscan < count; curscan++)
			polygon.m_callback(unit.scanline + curscan, unit.extent[curscan], *polygon.m_object, threadid);

		// set our count to 0 and re-fetch the original count value
		do
		{
			orig_count_next = unit.count_next;
		} while (compare_exchange32((volatile INT32 *)&unit.count_next, orig_count_next, 0) != orig_count_next);

		// if we have no more work to do, do nothing
		orig_count_next >>= 16;
		if (orig_count_next == 0)
			break;
		param = &polygon.m_owner->m_unit[orig_count_next];
	}
	return nullptr;
}


//-------------------------------------------------
//  wait - stall until all work is complete
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
void poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::wait(const char *debug_reason)
{
	osd_ticks_t time;

	// remember the start time if we're logging
	if (LOG_WAITS)
		time = get_profile_ticks();

	// wait for all pending work items to complete
	if (m_queue != nullptr)
		osd_work_queue_wait(m_queue, osd_ticks_per_second() * 100);

	// if we don't have a queue, just run the whole list now
	else
		for (int unitnum = 0; unitnum < m_unit.count(); unitnum++)
			work_item_callback(&m_unit[unitnum], 0);

	// log any long waits
	if (LOG_WAITS)
	{
		time = get_profile_ticks() - time;
		if (time > LOG_WAIT_THRESHOLD)
			machine().logerror("Poly:Waited %d cycles for %s\n", (int)time, debug_reason);
	}

	// reset the state
	m_polygon.reset();
	m_unit.reset();
	memset(m_unit_bucket, 0xff, sizeof(m_unit_bucket));

	// we need to preserve the last object data that was supplied
	if (m_object.count() > 0)
	{
		_ObjectData temp = object_data_last();
		m_object.reset();
		m_object.next() = temp;
	}
	else
		m_object.reset();
}


//-------------------------------------------------
//  object_data_alloc - allocate a new _ObjectData
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
_ObjectData &poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::object_data_alloc()
{
	// wait for a work item if we have to, then return the next item
	m_object.wait_for_space();
	return m_object.next();
}


//-------------------------------------------------
//  render_tile - render a tile
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_tile(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t &_v1, const vertex_t &_v2)
{
	const vertex_t *v1 = &_v1;
	const vertex_t *v2 = &_v2;

	// first sort by Y
	if (v2->y < v1->y)
	{
		const vertex_t *tv = v1;
		v1 = v2;
		v2 = tv;
	}

	// compute some integral X/Y vertex values
	INT32 v1y = round_coordinate(v1->y);
	INT32 v2y = round_coordinate(v2->y);

	// clip coordinates
	INT32 v1yclip = v1y;
	INT32 v2yclip = v2y + ((m_flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	v1yclip = MAX(v1yclip, cliprect.min_y);
	v2yclip = MIN(v2yclip, cliprect.max_y + 1);
	if (v2yclip - v1yclip <= 0)
		return 0;

	// determine total X extents
	_BaseType minx = v1->x;
	_BaseType maxx = v2->x;
	if (minx > maxx)
		return 0;

	// allocate and populate a new polygon
	polygon_info &polygon = polygon_alloc(round_coordinate(minx), round_coordinate(maxx), v1yclip, v2yclip, callback);

	// compute parameter deltas
	_BaseType param_dpdx[_MaxParams];
	_BaseType param_dpdy[_MaxParams];
	if (paramcount > 0)
	{
		_BaseType oox = poly_recip(v2->x - v1->x);
		_BaseType ooy = poly_recip(v2->y - v1->y);
		for (int paramnum = 0; paramnum < paramcount; paramnum++)
		{
			param_dpdx[paramnum]  = oox * (v2->p[paramnum] - v1->p[paramnum]);
			param_dpdy[paramnum]  = ooy * (v2->p[paramnum] - v1->p[paramnum]);
		}
	}

	// clamp to full pixels
	INT32 istartx = round_coordinate(v1->x);
	INT32 istopx = round_coordinate(v2->x);

	// force start < stop
	if (istartx > istopx)
	{
		INT32 temp = istartx;
		istartx = istopx;
		istopx = temp;
	}

	// include the right edge if requested
	if (m_flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
		istopx++;

	// apply left/right clipping
	if (istartx < cliprect.min_x)
		istartx = cliprect.min_x;
	if (istopx > cliprect.max_x)
		istopx = cliprect.max_x + 1;
	if (istartx >= istopx)
		return 0;

	// compute the X extents for each scanline
	INT32 pixels = 0;
	UINT32 startunit = m_unit.count();
	INT32 scaninc = 1;
	for (INT32 curscan = v1yclip; curscan < v2yclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.polygon = &polygon;
		unit.count_next = MIN(v2yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// compute the ending X based on which part of the triangle we're in
			_BaseType fully = _BaseType(curscan + extnum) + _BaseType(0.5);

			// set the extent and update the total pixel count
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;
			extent.userdata = NULL;
			pixels += istopx - istartx;

			// fill in the parameters for the extent
			_BaseType fullstartx = _BaseType(istartx) + _BaseType(0.5);
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				extent.param[paramnum].start = v1->p[paramnum] + fullstartx * param_dpdx[paramnum] + fully * param_dpdy[paramnum];
				extent.param[paramnum].dpdx = param_dpdx[paramnum];
			}
		}
	}

	// enqueue the work items
	if (m_queue != NULL)
		osd_work_item_queue_multiple(m_queue, work_item_callback, m_unit.count() - startunit, &m_unit[startunit], m_unit.itemsize(), WORK_ITEM_FLAG_AUTO_RELEASE);

	// return the total number of pixels in the triangle
	m_tiles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_triangle - render a single triangle
//  given 3 vertexes
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_triangle(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t &_v1, const vertex_t &_v2, const vertex_t &_v3)
{
	const vertex_t *v1 = &_v1;
	const vertex_t *v2 = &_v2;
	const vertex_t *v3 = &_v3;

	// first sort by Y
	if (v2->y < v1->y)
	{
		const vertex_t *tv = v1;
		v1 = v2;
		v2 = tv;
	}
	if (v3->y < v2->y)
	{
		const vertex_t *tv = v2;
		v2 = v3;
		v3 = tv;
		if (v2->y < v1->y)
		{
			const vertex_t *tv2 = v1;
			v1 = v2;
			v2 = tv2;
		}
	}

	// compute some integral X/Y vertex values
	INT32 v1y = round_coordinate(v1->y);
	INT32 v3y = round_coordinate(v3->y);

	// clip coordinates
	INT32 v1yclip = v1y;
	INT32 v3yclip = v3y + ((m_flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	v1yclip = MAX(v1yclip, cliprect.min_y);
	v3yclip = MIN(v3yclip, cliprect.max_y + 1);
	if (v3yclip - v1yclip <= 0)
		return 0;

	// determine total X extents
	_BaseType minx = v1->x;
	_BaseType maxx = v1->x;
	if (v2->x < minx) minx = v2->x;
	else if (v2->x > maxx) maxx = v2->x;
	if (v3->x < minx) minx = v3->x;
	else if (v3->x > maxx) maxx = v3->x;

	// allocate and populate a new polygon
	polygon_info &polygon = polygon_alloc(round_coordinate(minx), round_coordinate(maxx), v1yclip, v3yclip, callback);

	// compute the slopes for each portion of the triangle
	_BaseType dxdy_v1v2 = (v2->y == v1->y) ? _BaseType(0.0) : (v2->x - v1->x) / (v2->y - v1->y);
	_BaseType dxdy_v1v3 = (v3->y == v1->y) ? _BaseType(0.0) : (v3->x - v1->x) / (v3->y - v1->y);
	_BaseType dxdy_v2v3 = (v3->y == v2->y) ? _BaseType(0.0) : (v3->x - v2->x) / (v3->y - v2->y);

	// compute parameter starting points and deltas
	_BaseType param_start[_MaxParams];
	_BaseType param_dpdx[_MaxParams];
	_BaseType param_dpdy[_MaxParams];
	if (paramcount > 0)
	{
		_BaseType a00 = v2->y - v3->y;
		_BaseType a01 = v3->x - v2->x;
		_BaseType a02 = v2->x*v3->y - v3->x*v2->y;
		_BaseType a10 = v3->y - v1->y;
		_BaseType a11 = v1->x - v3->x;
		_BaseType a12 = v3->x*v1->y - v1->x*v3->y;
		_BaseType a20 = v1->y - v2->y;
		_BaseType a21 = v2->x - v1->x;
		_BaseType a22 = v1->x*v2->y - v2->x*v1->y;
		_BaseType det = a02 + a12 + a22;

		if (poly_abs(det) < _BaseType(0.00001))
		{
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				param_dpdx[paramnum] = _BaseType(0.0);
				param_dpdy[paramnum] = _BaseType(0.0);
				param_start[paramnum] = v1->p[paramnum];
			}
		}
		else
		{
			_BaseType idet = poly_recip(det);
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				param_dpdx[paramnum]  = idet * (v1->p[paramnum]*a00 + v2->p[paramnum]*a10 + v3->p[paramnum]*a20);
				param_dpdy[paramnum]  = idet * (v1->p[paramnum]*a01 + v2->p[paramnum]*a11 + v3->p[paramnum]*a21);
				param_start[paramnum] = idet * (v1->p[paramnum]*a02 + v2->p[paramnum]*a12 + v3->p[paramnum]*a22);
			}
		}
	}
	else    // GCC 4.7.0 incorrectly claims these are uninitialized; humor it by initializing in the (hopefully rare) zero parameter case
	{
		param_start[0] = _BaseType(0.0);
		param_dpdx[0] = _BaseType(0.0);
		param_dpdy[0] = _BaseType(0.0);
	}

	// compute the X extents for each scanline
	INT32 pixels = 0;
	UINT32 startunit = m_unit.count();
	INT32 scaninc = 1;
	for (INT32 curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.polygon = &polygon;
		unit.count_next = MIN(v3yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// compute the ending X based on which part of the triangle we're in
			_BaseType fully = _BaseType(curscan + extnum) + _BaseType(0.5);
			_BaseType startx = v1->x + (fully - v1->y) * dxdy_v1v3;
			_BaseType stopx;
			if (fully < v2->y)
				stopx = v1->x + (fully - v1->y) * dxdy_v1v2;
			else
				stopx = v2->x + (fully - v2->y) * dxdy_v2v3;

			// clamp to full pixels
			INT32 istartx = round_coordinate(startx);
			INT32 istopx = round_coordinate(stopx);

			// force start < stop
			if (istartx > istopx)
			{
				INT32 temp = istartx;
				istartx = istopx;
				istopx = temp;
			}

			// include the right edge if requested
			if (m_flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
				istopx++;

			// apply left/right clipping
			if (istartx < cliprect.min_x)
				istartx = cliprect.min_x;
			if (istopx > cliprect.max_x)
				istopx = cliprect.max_x + 1;

			// set the extent and update the total pixel count
			if (istartx >= istopx)
				istartx = istopx = 0;
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;
			extent.userdata = nullptr;
			pixels += istopx - istartx;

			// fill in the parameters for the extent
			_BaseType fullstartx = _BaseType(istartx) + _BaseType(0.5);
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
			{
				extent.param[paramnum].start = param_start[paramnum] + fullstartx * param_dpdx[paramnum] + fully * param_dpdy[paramnum];
				extent.param[paramnum].dpdx = param_dpdx[paramnum];
			}
		}
	}

	// enqueue the work items
	if (m_queue != nullptr)
		osd_work_item_queue_multiple(m_queue, work_item_callback, m_unit.count() - startunit, &m_unit[startunit], m_unit.itemsize(), WORK_ITEM_FLAG_AUTO_RELEASE);

	// return the total number of pixels in the triangle
	m_triangles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_triangle_fan - render a set of
//  triangles in a fan
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_triangle_fan(const rectangle &cliprect, render_delegate callback, int paramcount, int numverts, const vertex_t *v)
{
	// iterate over vertices
	UINT32 pixels = 0;
	for (int vertnum = 2; vertnum < numverts; vertnum++)
		pixels += render_triangle(cliprect, callback, paramcount, v[0], v[vertnum - 1], v[vertnum]);
	return pixels;
}


//-------------------------------------------------
//  render_triangle_strip - render a set of
//  triangles in a strip
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_triangle_strip(const rectangle &cliprect, render_delegate callback, int paramcount, int numverts, const vertex_t *v)
{
	// iterate over vertices
	UINT32 pixels = 0;
	for (int vertnum = 2; vertnum < numverts; vertnum++)
		pixels += render_triangle(cliprect, callback, paramcount, v[vertnum - 2], v[vertnum - 1], v[vertnum]);
	return pixels;
}


//-------------------------------------------------
//  render_triangle_custom - perform a custom
//  render of an object, given specific extents
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_triangle_custom(const rectangle &cliprect, render_delegate callback, int startscanline, int numscanlines, const extent_t *extents)
{
	// clip coordinates
	INT32 v1yclip = MAX(startscanline, cliprect.min_y);
	INT32 v3yclip = MIN(startscanline + numscanlines, cliprect.max_y + 1);
	if (v3yclip - v1yclip <= 0)
		return 0;

	// allocate and populate a new polygon
	polygon_info &polygon = polygon_alloc(0, 0, v1yclip, v3yclip, callback);

	// compute the X extents for each scanline
	INT32 pixels = 0;
	UINT32 startunit = m_unit.count();
	INT32 scaninc = 1;
	for (INT32 curscan = v1yclip; curscan < v3yclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.polygon = &polygon;
		unit.count_next = MIN(v3yclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			const extent_t &srcextent = extents[(curscan + extnum) - startscanline];
			INT32 istartx = srcextent.startx, istopx = srcextent.stopx;

			// apply left/right clipping
			if (istartx < cliprect.min_x)
				istartx = cliprect.min_x;
			if (istartx > cliprect.max_x)
				istartx = cliprect.max_x + 1;
			if (istopx < cliprect.min_x)
				istopx = cliprect.min_x;
			if (istopx > cliprect.max_x)
				istopx = cliprect.max_x + 1;

			// set the extent and update the total pixel count
			extent_t &extent = unit.extent[extnum];
			extent.startx = istartx;
			extent.stopx = istopx;

			// fill in the parameters for the extent
			for (int paramnum = 0; paramnum < _MaxParams; paramnum++)
			{
				extent.param[paramnum].start = srcextent.param[paramnum].start;
				extent.param[paramnum].dpdx = srcextent.param[paramnum].dpdx;
			}

			extent.userdata = srcextent.userdata;
			if (istartx < istopx)
				pixels += istopx - istartx;
			else if(istopx < istartx)
				pixels += istartx - istopx;
		}
	}

	// enqueue the work items
	if (m_queue != nullptr)
		osd_work_item_queue_multiple(m_queue, work_item_callback, m_unit.count() - startunit, &m_unit[startunit], m_unit.itemsize(), WORK_ITEM_FLAG_AUTO_RELEASE);

	// return the total number of pixels in the object
	m_triangles++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  render_polygon - render a single polygon up
//  to 32 vertices
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
template<int _NumVerts>
UINT32 poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::render_polygon(const rectangle &cliprect, render_delegate callback, int paramcount, const vertex_t *v)
{
	// determine min/max Y vertices
	_BaseType minx = v[0].x;
	_BaseType maxx = v[0].x;
	int minv = 0;
	int maxv = 0;
	for (int vertnum = 1; vertnum < _NumVerts; vertnum++)
	{
		if (v[vertnum].y < v[minv].y)
			minv = vertnum;
		else if (v[vertnum].y > v[maxv].y)
			maxv = vertnum;
		if (v[vertnum].x < minx)
			minx = v[vertnum].x;
		else if (v[vertnum].x > maxx)
			maxx = v[vertnum].x;
	}

	// determine start/end scanlines
	INT32 miny = round_coordinate(v[minv].y);
	INT32 maxy = round_coordinate(v[maxv].y);

	// clip coordinates
	INT32 minyclip = miny;
	INT32 maxyclip = maxy + ((m_flags & POLYFLAG_INCLUDE_BOTTOM_EDGE) ? 1 : 0);
	minyclip = MAX(minyclip, cliprect.min_y);
	maxyclip = MIN(maxyclip, cliprect.max_y + 1);
	if (maxyclip - minyclip <= 0)
		return 0;

	// allocate a new polygon
	polygon_info &polygon = polygon_alloc(round_coordinate(minx), round_coordinate(maxx), minyclip, maxyclip, callback);

	// walk forward to build up the forward edge list
	struct poly_edge
	{
		poly_edge *         next;                   // next edge in sequence
		int                 index;                  // index of this edge
		const vertex_t *    v1;                     // pointer to first vertex
		const vertex_t *    v2;                     // pointer to second vertex
		_BaseType           dxdy;                   // dx/dy along the edge
		_BaseType           dpdy[_MaxParams];       // per-parameter dp/dy values
	};
	poly_edge fedgelist[_NumVerts - 1];
	poly_edge *edgeptr = &fedgelist[0];
	for (int curv = minv; curv != maxv; curv = (curv == _NumVerts - 1) ? 0 : (curv + 1))
	{
		// set the two vertices
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == _NumVerts - 1) ? 0 : (curv + 1)];

		// if horizontal, skip altogether
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		// need dx/dy always, and parameter deltas as necessary
		_BaseType ooy = poly_recip(edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (int paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		++edgeptr;
	}

	// walk backward to build up the backward edge list
	poly_edge bedgelist[_NumVerts - 1];
	edgeptr = &bedgelist[0];
	for (int curv = minv; curv != maxv; curv = (curv == 0) ? (_NumVerts - 1) : (curv - 1))
	{
		// set the two vertices
		edgeptr->v1 = &v[curv];
		edgeptr->v2 = &v[(curv == 0) ? (_NumVerts - 1) : (curv - 1)];

		// if horizontal, skip altogether
		if (edgeptr->v1->y == edgeptr->v2->y)
			continue;

		// need dx/dy always, and parameter deltas as necessary
		_BaseType ooy = poly_recip(edgeptr->v2->y - edgeptr->v1->y);
		edgeptr->dxdy = (edgeptr->v2->x - edgeptr->v1->x) * ooy;
		for (int paramnum = 0; paramnum < paramcount; paramnum++)
			edgeptr->dpdy[paramnum] = (edgeptr->v2->p[paramnum] - edgeptr->v1->p[paramnum]) * ooy;
		++edgeptr;
	}

	// determine which list is left/right:
	// if the first vertex is shared, compare the slopes
	// if the first vertex is not shared, compare the X coordinates
	const poly_edge *ledge, *redge;
	if ((fedgelist[0].v1 == bedgelist[0].v1 && fedgelist[0].dxdy < bedgelist[0].dxdy) ||
		(fedgelist[0].v1 != bedgelist[0].v1 && fedgelist[0].v1->x < bedgelist[0].v1->x))
	{
		ledge = fedgelist;
		redge = bedgelist;
	}
	else
	{
		ledge = bedgelist;
		redge = fedgelist;
	}

	// compute the X extents for each scanline
	INT32 pixels = 0;
	UINT32 startunit = m_unit.count();
	INT32 scaninc = 1;
	for (INT32 curscan = minyclip; curscan < maxyclip; curscan += scaninc)
	{
		UINT32 bucketnum = ((UINT32)curscan / SCANLINES_PER_BUCKET) % TOTAL_BUCKETS;
		UINT32 unit_index = m_unit.count();
		work_unit &unit = m_unit.next();

		// determine how much to advance to hit the next bucket
		scaninc = SCANLINES_PER_BUCKET - (UINT32)curscan % SCANLINES_PER_BUCKET;

		// fill in the work unit basics
		unit.polygon = &polygon;
		unit.count_next = MIN(maxyclip - curscan, scaninc);
		unit.scanline = curscan;
		unit.previtem = m_unit_bucket[bucketnum];
		m_unit_bucket[bucketnum] = unit_index;

		// iterate over extents
		for (int extnum = 0; extnum < unit.count_next; extnum++)
		{
			// compute the ending X based on which part of the triangle we're in
			_BaseType fully = _BaseType(curscan + extnum) + _BaseType(0.5);
			while (fully > ledge->v2->y && fully < v[maxv].y)
				++ledge;
			while (fully > redge->v2->y && fully < v[maxv].y)
				++redge;
			_BaseType startx = ledge->v1->x + (fully - ledge->v1->y) * ledge->dxdy;
			_BaseType stopx = redge->v1->x + (fully - redge->v1->y) * redge->dxdy;

			// clamp to full pixels
			INT32 istartx = round_coordinate(startx);
			INT32 istopx = round_coordinate(stopx);

			// compute parameter starting points and deltas
			extent_t &extent = unit.extent[extnum];
			if (paramcount > 0)
			{
				_BaseType ldy = fully - ledge->v1->y;
				_BaseType rdy = fully - redge->v1->y;
				_BaseType oox = poly_recip(stopx - startx);

				// iterate over parameters
				for (int paramnum = 0; paramnum < paramcount; paramnum++)
				{
					_BaseType lparam = ledge->v1->p[paramnum] + ldy * ledge->dpdy[paramnum];
					_BaseType rparam = redge->v1->p[paramnum] + rdy * redge->dpdy[paramnum];
					_BaseType dpdx = (rparam - lparam) * oox;

					extent.param[paramnum].start = lparam;// - (_BaseType(istartx) + 0.5f) * dpdx;
					extent.param[paramnum].dpdx = dpdx;
				}
			}

			// include the right edge if requested
			if (m_flags & POLYFLAG_INCLUDE_RIGHT_EDGE)
				istopx++;

			// apply left/right clipping
			if (istartx < cliprect.min_x)
			{
				for (int paramnum = 0; paramnum < paramcount; paramnum++)
					extent.param[paramnum].start += (cliprect.min_x - istartx) * extent.param[paramnum].dpdx;
				istartx = cliprect.min_x;
			}
			if (istopx > cliprect.max_x)
				istopx = cliprect.max_x + 1;

			// set the extent and update the total pixel count
			if (istartx >= istopx)
				istartx = istopx = 0;
			extent.startx = istartx;
			extent.stopx = istopx;
			extent.userdata = nullptr;
			pixels += istopx - istartx;
		}
	}

	// enqueue the work items
	if (m_queue != nullptr)
		osd_work_item_queue_multiple(m_queue, work_item_callback, m_unit.count() - startunit, &m_unit[startunit], m_unit.itemsize(), WORK_ITEM_FLAG_AUTO_RELEASE);

	// return the total number of pixels in the triangle
	m_quads++;
	m_pixels += pixels;
	return pixels;
}


//-------------------------------------------------
//  zclip_if_less - clip a polygon using p[0] as
//  a z coordinate
//-------------------------------------------------

template<typename _BaseType, class _ObjectData, int _MaxParams, int _MaxPolys>
int poly_manager<_BaseType, _ObjectData, _MaxParams, _MaxPolys>::zclip_if_less(int numverts, const vertex_t *v, vertex_t *outv, int paramcount, _BaseType clipval)
{
	bool prevclipped = (v[numverts - 1].p[0] < clipval);
	vertex_t *nextout = outv;

	// iterate over vertices
	for (int vertnum = 0; vertnum < numverts; vertnum++)
	{
		bool thisclipped = (v[vertnum].p[0] < clipval);

		// if we switched from clipped to non-clipped, interpolate a vertex
		if (thisclipped != prevclipped)
		{
			const vertex_t &v1 = v[(vertnum == 0) ? (numverts - 1) : (vertnum - 1)];
			const vertex_t &v2 = v[vertnum];
			_BaseType frac = (clipval - v1.p[0]) / (v2.p[0] - v1.p[0]);
			nextout->x = v1.x + frac * (v2.x - v1.x);
			nextout->y = v1.y + frac * (v2.y - v1.y);
			for (int paramnum = 0; paramnum < paramcount; paramnum++)
				nextout->p[paramnum] = v1.p[paramnum] + frac * (v2.p[paramnum] - v1.p[paramnum]);
			++nextout;
		}

		// if this vertex is not clipped, copy it in
		if (!thisclipped)
			*nextout++ = v[vertnum];

		// remember the last state
		prevclipped = thisclipped;
	}
	return nextout - outv;
}


template<typename _BaseType, int _MaxParams>
struct frustum_clip_vertex
{
	_BaseType x, y, z, w;       // A 3d coordinate already transformed by a projection matrix
	_BaseType p[_MaxParams];    // Additional parameters to clip
};


template<typename _BaseType, int _MaxParams>
int frustum_clip_w(const frustum_clip_vertex<_BaseType, _MaxParams>* v, int num_vertices, frustum_clip_vertex<_BaseType, _MaxParams>* out)
{
	if (num_vertices <= 0)
		return 0;

	const _BaseType W_PLANE = 0.000001f;

	frustum_clip_vertex<_BaseType, _MaxParams> clipv[10];
	int clip_verts = 0;

	int previ = num_vertices - 1;

	for (int i=0; i < num_vertices; i++)
	{
		int v1_side = (v[i].w < W_PLANE) ? -1 : 1;
		int v2_side = (v[previ].w < W_PLANE) ? -1 : 1;

		if ((v1_side * v2_side) < 0)        // edge goes through W plane
		{
			// insert vertex at intersection point
			_BaseType wdiv = v[previ].w - v[i].w;
			if (wdiv == 0.0f)       // 0 edge means degenerate polygon
				return 0;

			_BaseType t = fabs((W_PLANE - v[previ].w) / wdiv);

			clipv[clip_verts].x = v[previ].x + ((v[i].x - v[previ].x) * t);
			clipv[clip_verts].y = v[previ].y + ((v[i].y - v[previ].y) * t);
			clipv[clip_verts].z = v[previ].z + ((v[i].z - v[previ].z) * t);
			clipv[clip_verts].w = v[previ].w + ((v[i].w - v[previ].w) * t);

			// Interpolate the rest of the parameters
			for (int pi = 0; pi < _MaxParams; pi++)
				clipv[clip_verts].p[pi] = v[previ].p[pi] + ((v[i].p[pi] - v[previ].p[pi]) * t);

			++clip_verts;
		}
		if (v1_side > 0)                // current point is inside
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}

	memcpy(&out[0], &clipv[0], sizeof(out[0]) * clip_verts);
	return clip_verts;
}


template<typename _BaseType, int _MaxParams>
int frustum_clip(const frustum_clip_vertex<_BaseType, _MaxParams>* v, int num_vertices, frustum_clip_vertex<_BaseType, _MaxParams>* out, int axis, int sign)
{
	if (num_vertices <= 0)
		return 0;

	frustum_clip_vertex<_BaseType, _MaxParams> clipv[10];
	int clip_verts = 0;

	int previ = num_vertices - 1;

	for (int i=0; i < num_vertices; i++)
	{
		int v1_side, v2_side;
		_BaseType* v1a = (_BaseType*)&v[i];
		_BaseType* v2a = (_BaseType*)&v[previ];

		_BaseType v1_axis, v2_axis;

		if (sign)       // +axis
		{
			v1_axis = v1a[axis];
			v2_axis = v2a[axis];
		}
		else            // -axis
		{
			v1_axis = -v1a[axis];
			v2_axis = -v2a[axis];
		}

		v1_side = (v1_axis <= v[i].w) ? 1 : -1;
		v2_side = (v2_axis <= v[previ].w) ? 1 : -1;

		if ((v1_side * v2_side) < 0)        // edge goes through W plane
		{
			// insert vertex at intersection point
			_BaseType wdiv = ((v[previ].w - v2_axis) - (v[i].w - v1_axis));

			if (wdiv == 0.0f)           // 0 edge means degenerate polygon
				return 0;

			_BaseType t = fabs((v[previ].w - v2_axis) / wdiv);

			clipv[clip_verts].x = v[previ].x + ((v[i].x - v[previ].x) * t);
			clipv[clip_verts].y = v[previ].y + ((v[i].y - v[previ].y) * t);
			clipv[clip_verts].z = v[previ].z + ((v[i].z - v[previ].z) * t);
			clipv[clip_verts].w = v[previ].w + ((v[i].w - v[previ].w) * t);

			// Interpolate the rest of the parameters
			for (int pi = 0; pi < _MaxParams; pi++)
				clipv[clip_verts].p[pi] = v[previ].p[pi] + ((v[i].p[pi] - v[previ].p[pi]) * t);

			++clip_verts;
		}
		if (v1_side > 0)                // current point is inside
		{
			clipv[clip_verts] = v[i];
			++clip_verts;
		}

		previ = i;
	}

	memcpy(&out[0], &clipv[0], sizeof(out[0]) * clip_verts);
	return clip_verts;
}


template<typename _BaseType, int _MaxParams>
int frustum_clip_all(frustum_clip_vertex<_BaseType, _MaxParams>* clip_vert, int num_vertices, frustum_clip_vertex<_BaseType, _MaxParams>* out)
{
	num_vertices = frustum_clip_w<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert);
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 0, 0);      // W <= -X
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 0, 1);      // W <= +X
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 1, 0);      // W <= -Y
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 1, 1);      // W <= +X
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 2, 0);      // W <= -Z
	num_vertices = frustum_clip<_BaseType, _MaxParams>(clip_vert, num_vertices, clip_vert, 2, 1);      // W <= +Z
	out = clip_vert;
	return num_vertices;
}


#endif  // __POLY_H__
